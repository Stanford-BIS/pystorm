from kivy.app import App
from kivy.event import EventDispatcher
from kivy.lang import Builder
from kivy.properties import ObjectProperty, ListProperty, StringProperty, \
    NumericProperty, Clock, partial
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.stacklayout import StackLayout
from kivy.uix.textinput import TextInput
from kivy.uix.widget import Widget
from kivy.uix.label import Label
from kivy.metrics import sp
import os
import threading
import queue
from io import StringIO
import sys
import traceback
import numpy as np

EQ = queue.Queue(1)

def _truncate_pwd():
    import re
    _pwd = os.getcwd()
    _home = os.path.expanduser('~')
    _pwd = re.sub(_home, "~", _pwd)
    _paths = _pwd.split('/')
    if len(_paths) > 4:
        _pwd = "%s/%s/.../%s/%s" % (_paths[0], _paths[1], _paths[-2], _paths[-1])
    return _pwd


class Shell(EventDispatcher):
    # , 'on_stop', 'on_start', 'on_complete', 'on_error'
    __events__ = ('on_output', 'on_complete')

    def run_command(self, command, show_output=True, *args):
        _old_stdout = sys.stdout
        _old_stderr = sys.stderr
        _redirected_output = sys.stdout = StringIO()
        _redirected_error = sys.stderr = StringIO()
        _success = True
        try:
            eval(compile(command, '<string>', 'single'))
        except:
            try:
                _exc_info = sys.exc_info()
            finally:
                _success = False
                traceback.print_exception(*_exc_info, file=_redirected_error)
                del _exc_info
        sys.stdout = _old_stdout
        sys.stderr = _old_stderr

        output = ""
        for line in _redirected_output.getvalue().splitlines(True):
            output += line
            if show_output:
                self.dispatch('on_output', line)
        if not _success:
            for line in _redirected_error.getvalue().splitlines(True):
                output += line
                if show_output:
                    self.dispatch('on_output', line)
        self.dispatch('on_complete', output)

    def stop(self, *args):
        self._do_run = False

    def main_loop(self, *args, **kwargs):
        """
        Main REPL loop
        """
        while(self._do_run):
            if not EQ.empty():
                _item = EQ.get()
                self.run_command(_item)

    def init(self, *args, **kwargs):
        self._do_run = True
        self.run_thread = threading.Thread(target=self.main_loop, args=args, kwargs=kwargs)
        self.run_thread.start()


class ConsoleInput(TextInput):
    '''Displays Output and sends input to Shell. Emits 'on_ready_to_input'
       when it is ready to get input from user.
    '''

    shell = ObjectProperty(None)
    '''Instance of KivyConsole(parent) widget
    '''
    current_prompt = ObjectProperty(None)
    '''Active prompt
    '''

    def __init__(self, **kwargs):
        super(ConsoleInput, self).__init__(**kwargs)

    def keyboard_on_key_down(self, window, keycode, text, modifiers):
        '''Override of _keyboard_on_key_down.
        '''
        if keycode[0] == 13:
            # Enter -> execute the command
            text = self.text
            if text.strip():
                Clock.schedule_once(partial(self._run_cmd, text))
            else:
                Clock.schedule_once(self.prompt)
        elif keycode[0] in [8, 37, 38]:
            self.cancel_selection()
        elif keycode[0] == 99 and modifiers == ['ctrl']:
            Clock.schedule_once(self.prompt)

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

    def prompt(self, *args):
        '''Show the PS1 variable
        '''
        self.shell.new_prompt()

class ConsolePrompt(BoxLayout):
    ps1_text = StringProperty('>')
    font_name = StringProperty('droid-sans-mono.ttf')
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

    console_input = ObjectProperty(None)
    '''Instance of BoxLayout
       :data:`console_input` is a :class:`~kivy.properties.ObjectProperty`
    '''

    scroll_view = ObjectProperty(None)
    '''Instance of :class:`~kivy.uix.scrollview.ScrollView`
       :data:`scroll_view` is a :class:`~kivy.properties.ObjectProperty`
    '''

    foreground_color = ListProperty((1, 1, 1, 1))
    '''This defines the color of the text in the console

    :data:`foreground_color` is an :class:`~kivy.properties.ListProperty`,
    Default to '(.5, .5, .5, .93)'
    '''

    background_color = ListProperty((0, 0, 0, 1))
    '''This defines the color of the text in the console

    :data:`foreground_color` is an :class:`~kivy.properties.ListProperty`,
    Default to '(0, 0, 0, 1)'''

    font_name = StringProperty('droid-sans-mono.ttf')
    '''Indicates the font Style used in the console

    :data:`font` is a :class:`~kivy.properties.StringProperty`,
    Default to 'DroidSansMono'
    '''

    font_size = NumericProperty(sp(28))
    '''Indicates the size of the font used for the console

    :data:`font_size` is a :class:`~kivy.properties.NumericProperty`,
    Default to '9'
    '''

    def __init__(self, **kwargs):
        super(KivyConsole, self).__init__(**kwargs)
        self.init()

        self._prompt = None
        self._spacer = None
        self.new_prompt()

        Clock.schedule_once(partial(self._prompt.init, shell=self))

    def on_output(self, output):
        '''Event handler to send output data
        '''
        _label = Label(size=(self.size[0], sp(20)), halign='left', valign='top', size_hint=(1, None), markup=True)
        _label.color = (0.7, 0.7, 0.7, 1)
        _label.text = '[i]' + output + '[/i]'
        _label.font_name = self.font_name
        _label.text_size = _label.size

        self.append_widget(_label)

    def append_widget(self, widget):
        if self._spacer is not None:
            self.console_input.remove_widget(self._spacer)
        self._spacer = Widget()
        self.console_input.add_widget(widget)
        self.console_input.add_widget(self._spacer)

    def new_prompt(self):
        if self._prompt is not None:
            self._prompt.prompt_input.readonly = True
            self._prompt.prompt_input.cursor_width = 0
        self._prompt = ConsolePrompt()
        self._prompt.prompt_ps1.text = '[i]' + _truncate_pwd() + '>>> [/i]'
        self._prompt.shell = self
        self.append_widget(self._prompt)
        Clock.schedule_once(partial(self._prompt.init, shell=self))
        self.scroll_view.scroll_y = 0
        
    def on_complete(self, output):
        '''Event handler to send output data
        '''
        self.new_prompt()

class ConsoleApp(App):
    def build(self):
        self.root = KivyConsole()
        return self.root
    
    def on_stop(self, *args, **kwargs):
        '''Event handler to clean-up
        '''
        self.root.stop()
        self.root.run_thread.join()

if __name__ == '__main__':
    _gui = ConsoleApp()
    _gui.run()
