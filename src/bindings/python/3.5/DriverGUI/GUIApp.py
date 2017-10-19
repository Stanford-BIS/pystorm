import sys
#import os
#os.environ['KIVY_DPI'] = '227'
#os.environ['KIVY_METRICS_DENSITY'] = '1'
from kivy.app import App
from kivy.app import Builder
#from kivy.metrics import sp, dp
from kivy.uix.actionbar import ActionBar
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.graphics.texture import Texture
from kivy.graphics import Rectangle
from kivy.uix.widget import Widget
from kivy.uix.gridlayout import GridLayout
from kivy.uix.anchorlayout import AnchorLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.stacklayout import StackLayout
from kivy.uix.label import Label
from kivy.uix.slider import Slider
from kivy.clock import Clock
from kivy.properties import ObjectProperty
from kivy.properties import NumericProperty
from kivy.config import Config
from kivy.uix.button import Button
from kivy.uix.textinput import TextInput
from kivy.uix.tabbedpanel import TabbedPanel
from kivy.uix.scatter import Scatter
from kivy.uix.stencilview import StencilView
from kivy.core.window import Window
from math import ceil
import threading
import time

import numpy as np

Window.size = (800, 540)

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
        self.sparkle = Rectangle()

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
        self.sparkle.pos = self.pos
        self.sparkle.size = self.size
        self.sparkle.texture = self.texture
        self.canvas.clear()
        self.canvas.add(self.sparkle)

        np.less(self.arr_data, self.DECAY_MAT, self._mask2)
        np.greater_equal(self.arr_data, self.DECAY_MAT, self._mask3)

        np.copyto(self.arr_data, self.ZERO_MAT, where=self._mask2)
        np.subtract(self.arr_data, self.DECAY_MAT, self.arr_data, where=self._mask3)

    def on_size(self, *args):
        self.canvas.clear()

    def on_pos(self, *args):
        self.canvas.clear()

class Raster(Widget):
    NUM_X_PIXELS = 1000
    NUM_Y_PIXELS = 4096
    REFRESH_PERIOD = 1./60.  # seconds
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

        # Trigger animation
        Clock.schedule_interval(self.update, self.REFRESH_PERIOD)

        ## Trigger Polling
        Clock.schedule_interval(self.poll_data, self.POLLING_PERIOD)

    def poll_data(self, dt):
        bin_rem = int(ceil(dt / self._bin_time))
        if bin_rem >= self.NUM_X_PIXELS:
            bin_rem = self.NUM_X_PIXELS - 1
            print("dt > window")
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
    #raster_stencil = ObjectProperty(None)
    raster_widget = ObjectProperty(None)

    def init_children(self):
        self.sparkle_widget.init()
        self.raster_widget.init()
        #self.raster_scatter.pos = self.raster_stencil.pos
        #self.raster_scatter.rotation = -90

    def reset_sparkle(self):
        self.sparkle_scatter.scale = 1
        self.sparkle_scatter.pos = self.sparkle_stencil.pos
        self.raster_scatter.scale = 1
        
    def update_fader(self, *args):
        self.sparkle_widget.__update_fade_vals__(args[0])

    def update_rasterwin(self, *args):
        self.raster_widget.__update_win_period__(args[0])

    def update_bias(self, slider, val_text, *args, **kwargs):
        val_text.text = str(slider.value)
        print("Name: %s, Value: %g" % (kwargs['name'], kwargs['value']))
        
class MainScreen(Screen):
    main_area = ObjectProperty(None)
    
    def init_children(self):
        self.main_area.init_children()


class SettingsScreen(Screen):
    pass

class GUIApp(App):
    
    def build(self):
        self.root = BoxLayout(orientation='vertical')
        self.action_bar = TopBar()
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
        return self.root

    
    def on_start(self):
        self.scr_main.init_children()


if __name__ == '__main__':
    _gui = GUIApp()
    _gui.run()
