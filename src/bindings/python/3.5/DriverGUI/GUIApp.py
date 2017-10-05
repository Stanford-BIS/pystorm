from kivy.app import App
from kivy.graphics.texture import Texture
from kivy.graphics import Rectangle
from kivy.uix.widget import Widget
from kivy.uix.gridlayout import GridLayout
from kivy.clock import Clock
from kivy.properties import ObjectProperty
from kivy.config import Config
from kivy.uix.button import Button

import numpy as np

Config.set('graphics', 'width', '512')
Config.set('graphics', 'height', '256')


class Renderer(Widget):

    def init_update(self):
        # create a 64x64 texture, defaults to rgb / ubyte
        self.texture = Texture.create(size=(64, 64), colorfmt='luminance')

        # create numpy array to hold spike data
        self.arr_data = np.ones(4096, dtype=np.uint8)

        # Trigger animation
        Clock.schedule_interval(self.update, 1./60.)

    def update(self, dt):
        self.arr_data[...] = np.random.randint(0, 255, 4096, dtype=np.uint8)

        # then blit the buffer
        self.texture.blit_buffer(self.arr_data, colorfmt='luminance', bufferfmt='ubyte')

        # that's all ! you can use it in your graphics now :)
        # if self is a widget, you can do this
        with self.canvas:
            Rectangle(texture=self.texture, pos=self.pos, size=self.size)

    def on_size(self, *args):
        self.canvas.clear()

    def on_pos(self, *args):
        self.canvas.clear()


class RootLayout(GridLayout):

    renderer_widget = ObjectProperty(None)

    def init_children(self):
        self.renderer_widget.init_update()


class GUIApp(App):

    def build(self):
        root = RootLayout()
        root.init_children()
        return root


if __name__ == '__main__':
    GUIApp().run()