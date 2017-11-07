from kivy.app import App
from kivy.event import EventDispatcher
from kivy.lang import Builder
from kivy.properties import ObjectProperty, ListProperty, StringProperty, \
    NumericProperty, Clock, partial
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.textinput import TextInput
import os
import threading
import queue
from io import StringIO
import sys
import traceback
import numpy as np

EQ = queue.Queue(1)

Builder.load_string('''
<KivyConsole>:
    console_input: console_input
    scroll_view: scroll_view
    ScrollView:
        id: scroll_view
        size_hint: (1, None)
        size: (root.width, root.height)
        bar_width: 20
        scroll_type: ['bars', 'content']
        ConsoleInput:
            id: console_input
            shell: root
            size_hint_y: None
            font_name: root.font_name
            font_size: root.font_size
            foreground_color: root.foreground_color
            background_color: root.background_color
            height: max(scroll_view.height, self.minimum_height)
''')


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

    def __init__(self, **kwargs):
        super(ConsoleInput, self).__init__(**kwargs)
        self._cursor_pos = 0  # position of the cursor before after prompt
        self.cursor_width = 16
        self.__init_console()

    def __init_console(self, *args):
        '''Create initial values for the prompt and shows it
        '''
        Clock.schedule_once(self.prompt)

    def keyboard_on_key_down(self, window, keycode, text, modifiers):
        '''Override of _keyboard_on_key_down.
        '''
        Clock.schedule_once(self.validate_cursor_pos)

        if keycode[0] == 13:
            # Enter -> execute the command
            text = self.text[self._cursor_pos:]
            if text.strip():
                Clock.schedule_once(partial(self._run_cmd, text))
            else:
                Clock.schedule_once(self.prompt)
            self.parent.scroll_y = 0
        elif keycode[0] in [8, 37, 38]:
            self.cancel_selection()
            if self.cursor_index() <= self._cursor_pos:
                return False
        elif keycode[0] == 99 and modifiers == ['ctrl']:
            self.shell.dispatch('on_output', "\n")
            Clock.schedule_once(self.prompt)

        if self.cursor_index() < self._cursor_pos:
            return False

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


    def validate_cursor_pos(self, *args):
        if self.cursor_index() < self._cursor_pos:
            self.cursor = self.get_cursor_from_index(self._cursor_pos)

    @staticmethod
    def _truncate_pwd():
        import re
        _pwd = os.getcwd()
        _home = os.path.expanduser('~')
        _pwd = re.sub(_home, "~", _pwd)
        _paths = _pwd.split('/')
        if len(_paths) > 4:
            _pwd = "%s/%s/.../%s/%s" % (_paths[0], _paths[1], _paths[-2], _paths[-1])
        return _pwd

    def prompt(self, *args):
        '''Show the PS1 variable
        '''
        ps1 = "%s>>> " % self._truncate_pwd()
        self._cursor_pos = self.cursor_index() + len(ps1)
        self.text += ps1

    def on_cursor(self, *args, **kwargs):
        Clock.schedule_once(self.validate_cursor_pos)
        super(ConsoleInput, self).on_cursor(*args, **kwargs)

    def on_output(self, output):
        self.text += output
        # print(output)

    def on_complete(self, output):
        self.prompt()

class KivyConsole(BoxLayout, Shell):

    console_input = ObjectProperty(None)
    '''Instance of ConsoleInput
       :data:`console_input` is an :class:`~kivy.properties.ObjectProperty`
    '''

    scroll_view = ObjectProperty(None)
    '''Instance of :class:`~kivy.uix.scrollview.ScrollView`
       :data:`scroll_view` is an :class:`~kivy.properties.ObjectProperty`
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

    font_size = NumericProperty(28)
    '''Indicates the size of the font used for the console

    :data:`font_size` is a :class:`~kivy.properties.NumericProperty`,
    Default to '9'
    '''

    def __init__(self, **kwargs):
        super(KivyConsole, self).__init__(**kwargs)
        # self.shell = Shell()
        # self.run_command = self.shell.run_command
        # self.shell.bind(on_output=self.console_input.on_output)
        # self.shell.bind(on_complete=self.console_input.on_output)
        self.init()

    def on_output(self, output):
        '''Event handler to send output data
        '''
        self.console_input.on_output(output)

    def on_complete(self, output):
        '''Event handler to send output data
        '''
        self.console_input.on_complete(output)

class GUIApp(App):
    def build(self):
        self.root = KivyConsole()
        return self.root
    
    def on_stop(self, *args, **kwargs):
        '''Event handler to clean-up
        '''
        self.root.stop()
        self.root.run_thread.join()

if __name__ == '__main__':
    _gui = GUIApp()
    _gui.run()
