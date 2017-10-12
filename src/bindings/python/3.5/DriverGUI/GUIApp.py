import sys
from kivy.app import App
from kivy.app import Builder
from kivy.graphics.texture import Texture
from kivy.graphics import Rectangle
from kivy.uix.widget import Widget
from kivy.uix.gridlayout import GridLayout
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.label import Label
from kivy.uix.slider import Slider
from kivy.clock import Clock
from kivy.properties import ObjectProperty
from kivy.properties import NumericProperty
from kivy.config import Config
from kivy.uix.button import Button
from kivy.uix.tabbedpanel import TabbedPanel
import threading
import time

import numpy as np

Config.set('graphics', 'width', '1024')
Config.set('graphics', 'height', '580')


class Renderer(Widget):
    DECAY_PERIOD   = 0.1  # seconds
    REFRESH_PERIOD = 1./60.  # seconds
    DECAY_AMOUNT   = int(255 / DECAY_PERIOD * REFRESH_PERIOD)
    POLLING_PERIOD = 1e-3  # 100e-6
    DECAY_MAT      = np.full(4096, DECAY_AMOUNT, dtype=np.uint8)
    ZERO_MAT       = np.zeros(4096, dtype=np.uint8)
    MAX_MAT        = np.full(4096, 255, dtype=np.uint8)

    # create numpy array to hold spike data
    arr_data = np.zeros(4096, dtype=np.uint8)
    _mask1 = np.full(4096, False, dtype=np.bool)
    _mask2 = np.full(4096, False, dtype=np.bool)
    _mask3 = np.full(4096, False, dtype=np.bool)
    
    def __update_fade_vals__(self, decay_period):
        self.DECAY_PERIOD = decay_period
        self.DECAY_AMOUNT = int(255 / self.DECAY_PERIOD * self.REFRESH_PERIOD)
        self.DECAY_MAT = np.full(4096, self.DECAY_AMOUNT, dtype=np.uint8)

    def init(self):
        # create a 64x64 texture, defaults to rgb / ubyte
        self.texture = Texture.create(size=(64, 64), colorfmt='luminance')
        self.texture.mag_filter = 'nearest'
        self.raster = Rectangle()

        # Trigger animation
        Clock.schedule_interval(self.update, self.REFRESH_PERIOD)

        ## Trigger Polling
        Clock.schedule_interval(self.poll_data, self.POLLING_PERIOD)
        #threading.Thread(target=self.poll_data).start()

    def poll_data(self, *args):
        np.greater(np.random.rand(4096), 0.9, self._mask1)
        np.copyto(self.arr_data, self.MAX_MAT, where=self._mask1)

    def update(self, dt):
        # then blit the buffer
        self.texture.blit_buffer(self.arr_data, colorfmt='luminance', bufferfmt='ubyte')
        self.raster.pos = self.pos
        self.raster.size = self.size
        self.raster.texture = self.texture
        self.canvas.clear()
        self.canvas.add(self.raster)

        np.less(self.arr_data, self.DECAY_MAT, self._mask2)
        np.greater_equal(self.arr_data, self.DECAY_MAT, self._mask3)

        np.copyto(self.arr_data, self.ZERO_MAT, where=self._mask2)
        np.subtract(self.arr_data, self.DECAY_MAT, self.arr_data, where=self._mask3)

    def on_size(self, *args):
        self.canvas.clear()

    def on_pos(self, *args):
        self.canvas.clear()


class RootLayout(BoxLayout):

    renderer_widget = ObjectProperty(None)
    fade_slider = ObjectProperty(None)

    def init_children(self):
        self.renderer_widget.init()
        
    def update_fader(self, *args):
        self.renderer_widget.__update_fade_vals__(args[0])


class GUIApp(App):
    
    def build(self):
        self.root_layout = RootLayout()
        return self.root_layout
    
    def on_start(self):
        self.root_layout.init_children()


if __name__ == '__main__':
    import re
    app_file_name = sys.argv[0].rstrip()
    kv_file_name = re.sub(r'(.*)App.py$', r'\1.kv', app_file_name)
    _gui = GUIApp()
    _gui.load_kv(kv_file_name)
    _gui.run()