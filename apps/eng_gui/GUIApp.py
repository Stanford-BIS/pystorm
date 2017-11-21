import sys
import os
import threading
from multiprocessing import Process, Queue, Array
import queue
from io import StringIO
import traceback
from math import ceil, floor
import ctypes
import time

#os.environ['KIVY_DPI'] = '227'
#os.environ['KIVY_METRICS_DENSITY'] = '1'
from kivy.app import App
from kivy.event import EventDispatcher
from kivy.properties import ObjectProperty, ListProperty, StringProperty, \
    NumericProperty, Clock, partial
from kivy.uix.actionbar import ActionBar
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.graphics.texture import Texture
from kivy.graphics import Rectangle
from kivy.uix.widget import Widget
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.textinput import TextInput
from kivy.core.window import Window
from kivy.metrics import sp

import numpy as np

from pystorm.PyDriver import bddriver as bd

Window.size = (800, 640)

__g_refresh_period__ = 1./60.

# message queue for REPL
EQ = queue.Queue(1)

# message queue for IPC commands
CMDQ = Queue(1)

# message queue for IPC results
RESQ = Queue(1)

# message queue for IPC errors
ERRQ = Queue(1)

# message queue for IPC control
CTRLQ = Queue(1)

# namespace for the console interpreter
NS = {}

# BD driver
BDDriver = bd.Driver()

# Shared Sparkle plot data
__shared_arr__ = Array(ctypes.c_float, 4096)

def load(file_name):
    with open(file_name) as f:
        code = compile(f.read(), file_name, 'exec')
        exec(code, globals())


def __driver_process__(cmd_q, res_q, err_q, cntrl_q, g_dict, l_dict):
    _do_run = True
    REFRESH_PERIOD = __g_refresh_period__
    #POLL_PERIOD = 0.001
    POLL_PERIOD = 1./60.

    DECAY_PERIOD = 0.1
    DECAY_AMOUNT = 1.0 / DECAY_PERIOD * REFRESH_PERIOD * REFRESH_PERIOD / POLL_PERIOD
    DECAY_MAT = np.full(4096, DECAY_AMOUNT, dtype=np.float32)

    ZERO_MAT = np.zeros(4096, dtype=np.float32)
    #ARR_DATA = np.zeros(4096, dtype=np.float32)
    ARR_DATA = np.frombuffer(__shared_arr__.get_obj(), dtype=np.float32)
    MAX_MAT = np.full(4096, 1.0, dtype=np.float32)
    ZERO_MASK = np.full(4096, False, dtype=bool)
    DECAY_MASK = np.full(4096, False, dtype=bool)


    def __cmd_loop__():
        nonlocal _do_run
        nonlocal DECAY_PERIOD, DECAY_AMOUNT, DECAY_MAT
        _cntrl = ""
        while _do_run:
            # Buffer to capture output and errors
            _redirected_output = sys.stdout = StringIO()
            _redirected_error = sys.stderr = StringIO()

            #_redirected_output = StringIO()
            #_redirected_error = StringIO()

            # Check the control queue
            try:
                _cntrl = cntrl_q.get(False, 0)
            except queue.Empty:
                _cntrl = (None, )
            # If control signal says exit, kill the loop
            if _cntrl[0] == "__exit__":
                _do_run = False
                continue
            elif _cntrl[0] == "__decay_T__":
                DECAY_PERIOD = _cntrl[1]
                DECAY_AMOUNT = 1.0 / DECAY_PERIOD * REFRESH_PERIOD * REFRESH_PERIOD / POLL_PERIOD
                DECAY_MAT = np.full(4096, DECAY_AMOUNT, dtype=np.float32)

            # Check command queue
            try:
                _cmd = cmd_q.get(False, 1)
            except queue.Empty:
                continue

            try:
                eval(compile(_cmd, '<string>', 'single'), g_dict, l_dict)
            except:
                try:
                    _exc_info = sys.exc_info()
                finally:
                    traceback.print_exception(*_exc_info, file=_redirected_error)
                    del _exc_info

            res_q.put(_redirected_output.getvalue())
            err_q.put(_redirected_error.getvalue())

            #res_q.put("")
            #err_q.put("")

    def __data_loop__():
        _mask1 = np.full(4096, False, dtype=np.bool)
        _cnt = 0
        while _do_run:
            #spike_data = BDDriver.RecvXYSpikesMasked(0)
            #spike_idx = spike_data[0]
            #spike_t = spike_data[1]

            t_min = _cnt * POLL_PERIOD
            t_max = t_min + POLL_PERIOD
            spike_idx = np.zeros(4096, dtype=np.uint8)
            spike_t = np.zeros(4096, dtype=np.float)
            _rand = np.random.rand(4096)
            _valid_idx = np.where(_rand > 0.9)[0]
            spike_idx[_valid_idx] = 1
            spike_t[_valid_idx] = _rand[_valid_idx] * POLL_PERIOD + t_min

            _decay = MAX_MAT - DECAY_AMOUNT * (t_max - spike_t) / POLL_PERIOD

            np.subtract(ARR_DATA, DECAY_MAT, ARR_DATA)
            np.less(ARR_DATA, DECAY_MAT, ZERO_MASK)

            np.copyto(ARR_DATA, ZERO_MAT, where=ZERO_MASK)
            np.copyto(ARR_DATA, _decay, where=spike_idx.astype(bool))

            time.sleep(POLL_PERIOD)
            _cnt += 1

    cmd_thread = threading.Thread(target=__cmd_loop__)
    data_thread = threading.Thread(target=__data_loop__)

    cmd_thread.start()
    data_thread.start()

    cmd_thread.join()
    data_thread.join()

def _truncate_pwd():
    """
    Truncate path if it is more than 4 directories

    :return:
    """
    import re
    _pwd = os.getcwd()
    _home = os.path.expanduser('~')
    _pwd = re.sub(_home, "~", _pwd)
    _paths = _pwd.split('/')
    if len(_paths) > 4:
        _pwd = "%s/%s/.../%s/%s" % (_paths[0], _paths[1], _paths[-2], _paths[-1])
    return _pwd


class Shell(EventDispatcher):
    __events__ = ('on_output', 'on_complete')
    __cmd_hist__ = []
    __hist_item__ = 1


    def run_command(self, command, show_output=True, *args):
        CMDQ.put(command)
        self.__cmd_hist__.append(command)
        self.__hist_item__ = len(self.__cmd_hist__)
        _res = RESQ.get(True)
        _err = ERRQ.get(True)

        _success = True
        if _err != '':
            _success = False

        output = ""
        for line in _res.splitlines(True):
            output += line
            if show_output:
                self.dispatch('on_output', line)
        if not _success:
            for line in _err.splitlines(True):
                output += line
                if show_output:
                    self.dispatch('on_output', line, False)
        self.dispatch('on_complete', output)


    def stop(self, *args):
        self._do_run = False
        CMDQ.put("__exit__()")
        CTRLQ.put(("__exit__", ), True)

        self.driver_proc.join()
        self.run_thread.join()

    def main_loop(self, *args, **kwargs):
        """
        Main REPL loop
        """
        while(self._do_run):
            if not EQ.empty():
                _item = EQ.get()
                self.run_command(_item)

    def init(self, *args, **kwargs):
        self.driver_proc = Process(target=__driver_process__, args=(CMDQ, RESQ, ERRQ, CTRLQ, globals(), NS))

        self._do_run = True
        self.run_thread = threading.Thread(target=self.main_loop, args=args, kwargs=kwargs)
        self.driver_proc.start()
        self.run_thread.start()


    def new_prompt(self, *args):
        self.__hist_item__ = len(self.__cmd_hist__)

class ConsoleInput(TextInput):
    """
    Displays Output and sends input to Shell. Emits 'on_ready_to_input'
    when it is ready to get input from user.
    """

    # Instance of KivyConsole(parent) widget
    shell = ObjectProperty(None)

    # Active prompt
    current_prompt = ObjectProperty(None)

    def __init__(self, **kwargs):
        super(ConsoleInput, self).__init__(**kwargs)

    def keyboard_on_key_down(self, window, keycode, text, modifiers):
        # Override of _keyboard_on_key_down.

        if keycode[0] == 13:
            # Enter -> execute the command
            text = self.text
            if text.strip():
                Clock.schedule_once(partial(self._run_cmd, text))
            else:
                Clock.schedule_once(self.shell.new_prompt)
        elif keycode[0] in [8, 37, 38]:
            self.cancel_selection()
        elif keycode[0] == 99 and modifiers == ['ctrl']:
            Clock.schedule_once(self.shell.new_prompt)
        elif keycode[0] in [273, 38]:  # up arrow searches history, latest to oldest
            if self.shell.__hist_item__ > 1:
                self.shell.__hist_item__ -= 1
            if len(self.shell.__cmd_hist__) > self.shell.__hist_item__:
                self.text = self.shell.__cmd_hist__[self.shell.__hist_item__]
        elif keycode[0] in [274, 40]:  # down arrow searches history, oldest to latest
            if self.shell.__hist_item__ < (len(self.shell.__cmd_hist__) - 1):
                self.shell.__hist_item__ += 1
            if len(self.shell.__cmd_hist__) > self.shell.__hist_item__:
                self.text = self.shell.__cmd_hist__[self.shell.__hist_item__]

        return super(ConsoleInput, self).keyboard_on_key_down(
            window, keycode, text, modifiers)

    def _run_cmd(self, cmd, *args):
        commands = cmd.split("\n")
        _num = len(commands)
        _idx = 0
        while(_idx < _num):
            if not EQ.full():
                EQ.put(commands[_idx])
                _idx += 1

class ConsoleOutput(Label):
    pass

class ConsolePrompt(BoxLayout):
    ps1_text = StringProperty('>')
    font_name = StringProperty('Inconsolata-Regular.ttf')
    font_size = NumericProperty(28)

    prompt_ps1 = ObjectProperty(None)
    prompt_input = ObjectProperty(None)
    prompt_height = NumericProperty(sp(20))

    def __init__(self, **kwargs):
        super(ConsolePrompt, self).__init__(**kwargs)

    def init(self, *args, **kwargs):
        self.prompt_ps1.width = (len(self.prompt_ps1.text) - 7) * self.prompt_ps1.texture_size[0]
        self.prompt_input.width = self.width - self.prompt_ps1.width
        self.prompt_input.shell = kwargs['shell']
        self.prompt_input.focus = True

class KivyConsole(BoxLayout, Shell):

    """Instance of BoxLayout
       :data:`console_input` is a :class:`~kivy.properties.ObjectProperty`
    """
    console_area = ObjectProperty(None)

    """Instance of :class:`~kivy.uix.scrollview.ScrollView`
       :data:`scroll_view` is a :class:`~kivy.properties.ObjectProperty`
    """
    scroll_view = ObjectProperty(None)

    """
    This defines the color of the text in the console

    :data:`foreground_color` is an :class:`~kivy.properties.ListProperty`,
    Default to '(.5, .5, .5, .93)'
    """
    foreground_color = ListProperty((1, 1, 1, 1))

    """This defines the color of the text in the console

    :data:`foreground_color` is an :class:`~kivy.properties.ListProperty`,
    Default to '(0, 0, 0, 1)
    """
    background_color = ListProperty((0, 0, 0, 1))

    """Indicates the font Style used in the console

    :data:`font` is a :class:`~kivy.properties.StringProperty`,
    Default to 'Inconsolata'
    """
    font_name = StringProperty('Inconsolata-Regular.ttf')

    """Indicates the size of the font used for the console

    :data:`font_size` is a :class:`~kivy.properties.NumericProperty`,
    Default to '9'
    """
    font_size = NumericProperty(sp(28))

    def __init__(self, **kwargs):
        super(KivyConsole, self).__init__(**kwargs)
        self.init()

        self._prompt = None
        self._spacer = None
        self._output = None
        self._output_text = ""
        self._last_success = True
        self.new_prompt()

        Clock.schedule_once(partial(self._prompt.init, shell=self))

    def on_output(self, output, success=True):
        """
        Event handler to send output data
        """

        self._output_text += output
        self._last_success = success

    def append_widget(self, *args,  **kwargs):
        widget = kwargs['widget']
        if self._spacer is not None:
            self.console_area.remove_widget(self._spacer)
        self._spacer = Widget()
        self.console_area.add_widget(widget)
        self.console_area.add_widget(self._spacer)

    def new_prompt(self, *args):
        if self._prompt is not None:
            self._prompt.prompt_input.readonly = True
            self._prompt.prompt_input.cursor_width = 0
        self._prompt = ConsolePrompt()
        self._prompt.prompt_ps1.text = '[b]' + _truncate_pwd() + '>>> [/b]'
        self._prompt.shell = self
        self.append_widget(widget=self._prompt)
        Clock.schedule_once(partial(self._prompt.init, shell=self))
        self.scroll_view.scroll_y = 0
        self._output = None

        super(KivyConsole, self).new_prompt()

    def _parse_output(self):
        import re
        _lines = self._output_text.splitlines(True)
        if self._last_success is False:
            _lines[-1] = str(re.sub(r'^(\S+Error)', r'[color=ff0000]\1[/color]', _lines[-1]))
            return (_lines[0] + ''.join(_lines[3:])).strip()
        else:
            return self._output_text.strip()

    def on_complete(self, output):
        """
        Event handler to send output data

        :param output: Interpreter output
        :return:
        """
        if self._output_text != "":
            self._output_text = self._parse_output()
            self._output = ConsoleOutput(
                text=self._output_text,
                font_name=self.font_name,
                text_size=(self.width, None)
            )
            self.append_widget(widget=self._output)
            self._output_text = ""

        self.new_prompt()

class Separator(Widget):
    pass

class TopBar(ActionBar):
    pass

class SliderRow(BoxLayout):

    def update_slider(self, inst_s, inst_t, val):
        _val = int(val)
        if _val < 1:
            _val = 1
        elif _val > 1024:
            _val = 1024
        inst_s.value = _val
        inst_t.text = str(_val)

class Sparkle(Widget):
    REFRESH_PERIOD = __g_refresh_period__

    # create numpy array to hold spike data
    arr_data = np.frombuffer(__shared_arr__.get_obj(), dtype=np.float32)

    def init(self):
        # create a 64x64 texture, defaults to rgb / ubyte
        self.texture = Texture.create(size=(64, 64), colorfmt='luminance', bufferfmt='float')
        self.texture.mag_filter = 'nearest'
        self.sparkle = Rectangle()

    def update(self, dt):
        # then blit the buffer
        self.texture.blit_buffer(self.arr_data, colorfmt='luminance', bufferfmt='float')
        self.sparkle.pos = self.pos
        self.sparkle.size = self.size
        self.sparkle.texture = self.texture
        self.canvas.clear()
        self.canvas.add(self.sparkle)

    def on_size(self, *args):
        self.canvas.clear()

    def on_pos(self, *args):
        self.canvas.clear()

class Raster(Widget):
    NUM_X_PIXELS = 1000
    NUM_Y_PIXELS = 4096
    REFRESH_PERIOD = __g_refresh_period__ # seconds
    POLLING_PERIOD = 1e-3  # seconds
    ZERO_MAT       = np.zeros(4096, dtype=np.uint8)
    MAX_MAT        = np.full(4096, 255, dtype=np.uint8)
    window_size = 1.0  # Time window (s) to show the spikes over
    arr_data = np.zeros(4096, dtype=np.uint8)
    raster_data = np.zeros((NUM_X_PIXELS, NUM_Y_PIXELS), dtype=np.uint8)
    _mask1 = np.full(4096, False, dtype=np.bool)

    def __update_win_period__(self, win_period):
        self.window_size = win_period
        self._bin_time = self.window_size / self.NUM_X_PIXELS
        self._bin_num = int(ceil(self._bin_time / self.POLLING_PERIOD))
        self._remainder = self._bin_num

    def init(self):
        self.__update_win_period__(self.window_size)
        self.texture = Texture.create(size=(self.NUM_Y_PIXELS, self.NUM_X_PIXELS), colorfmt='luminance')
        self.texture.mag_filter = 'nearest'
        self.raster = Rectangle()

    def poll_data(self, dt):
        bin_rem = int(ceil(dt / self._bin_time))
        if bin_rem >= self.NUM_X_PIXELS:
            bin_rem = self.NUM_X_PIXELS - 1
        self._remainder -= bin_rem
        if(self._remainder <= 0):
            self._remainder = self._bin_num
            self.raster_data[:-bin_rem, :] = self.raster_data[bin_rem:, :]
            self.raster_data[-bin_rem:-1, :] = 0
            self.raster_data[-1, :] = self.arr_data
            np.copyto(self.arr_data, self.ZERO_MAT)

        np.greater(np.random.rand(4096), 0.9, self._mask1)
        np.copyto(self.arr_data, self.MAX_MAT, where=self._mask1)

    def update(self, dt):
        # then blit the buffer
        self.texture.blit_buffer(np.reshape(self.raster_data, -1), colorfmt='luminance', bufferfmt='ubyte')
        self.raster.pos = self.pos
        self.raster.size = self.size
        self.raster.texture = self.texture
        self.canvas.clear()
        self.canvas.add(self.raster)

class RootLayout(BoxLayout):

    ROW_HEIGHT = 60
    sparkle_box = ObjectProperty(None)
    sparkle_stencil = ObjectProperty(None)
    sparkle_widget = ObjectProperty(None)
    fade_slider = ObjectProperty(None)
    raster_box = ObjectProperty(None)
    raster_widget = ObjectProperty(None)

    def init_children(self):
        self.sparkle_widget.init()
        self.raster_widget.init()

        # Trigger animation
        self.sparkle_anim_event = Clock.schedule_interval(self.sparkle_widget.update, self.sparkle_widget.REFRESH_PERIOD)

        # Trigger animation
        self.raster_anim_event = None

        # Trigger Polling
        self.raster_poll_event = None

    def select_plot(self, type_name):
        if type_name == 'sparkle':
            if self.raster_poll_event is not None:
                self.raster_anim_event.cancel()
                self.raster_poll_event.cancel()
                self.raster_anim_event = None
                self.raster_poll_event = None
        elif type_name == 'raster':
            if self.raster_poll_event is None:
                self.raster_anim_event = Clock.schedule_interval(self.raster_widget.update, self.raster_widget.REFRESH_PERIOD)
                self.raster_poll_event = Clock.schedule_interval(self.raster_widget.poll_data, self.raster_widget.POLLING_PERIOD)

    def reset_sparkle(self):
        self.sparkle_scatter.scale = 1
        self.sparkle_scatter.pos = self.sparkle_stencil.pos

    def update_fader(self, *args):
        CTRLQ.put(("__decay_T__", args[0]))

    def update_rasterwin(self, *args):
        self.raster_widget.__update_win_period__(args[0])

    def update_bias(self, slider, val_text, *args, **kwargs):
        val_text.text = str(slider.value)
        print("Name: %s, Value: %g" % (kwargs['name'], kwargs['value']))

class MainScreen(Screen):
    main_area = ObjectProperty(None)

    def init_children(self):
        self.main_area.init_children()

class SettingsLayout(BoxLayout):
    def init_children(self):
        pass

class SettingsScreen(Screen):
    settings_area = ObjectProperty(None)

    def init_children(self):
        self.settings_area.init_children()


def InitBD():
    print("[INFO] Starting BD")
    BDDriver.Start() # starts BDDriver threads
    print("[INFO] Resetting BD")
    BDDriver.ResetBD()
    print("[INFO] Init the FIFO (also turns on traffic)")
    BDDriver.InitFIFO(0)
    print("[INFO] BDDriver available as '[color=ffffff]BDDriver[/color]'")


class GUIApp(App):

    def build(self):
        self.root = BoxLayout(orientation='vertical')
        self.action_bar = TopBar()
        self.console = KivyConsole()
        self.console.height = sp(100)
        self.console.size_hint_y = None

        self.sm = ScreenManager()
        self.scr_main = MainScreen(name='main')
        self.scr_settings = SettingsScreen(name='settings')

        def __select_screen__(scene_name):
            self.sm.current = scene_name

        self.action_bar.scr_cb = __select_screen__

        self.root.add_widget(self.action_bar)
        self.sm.add_widget(self.scr_main)
        self.sm.add_widget(self.scr_settings)
        self.root.add_widget(self.sm)
        self.root.add_widget(Separator())
        self.root.add_widget(self.console)
        return self.root


    def on_start(self):
        self.scr_main.init_children()
        Clock.schedule_once(partial(self.console.run_command, "InitBD()"))


    def on_stop(self, *args, **kwargs):
        """
        Event handler to clean-up

        :param args:
        :param kwargs:
        :return:
        """
        self.console.stop()

if __name__ == '__main__':
    _gui = GUIApp()
    _gui.run()
