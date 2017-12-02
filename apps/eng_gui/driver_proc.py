"""
This module contains the driver process' loop
"""
import numpy as np
import threading
import sys
from io import StringIO
import traceback
import queue
import time
from pystorm.PyDriver import bddriver as bd

__g_refresh_period__ = None
__shared_arr__ = None

def __proc__(cmd_q, res_q, err_q, cntrl_q, cntrl_r, g_dict, l_dict):
    driver = bd.Driver()
    print(driver)
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

    def load(file_name):
        with open(file_name) as f:
            code = compile(f.read(), file_name, 'exec')
            eval(code, proc_dict, proc_dict)

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
            elif _cntrl[0] == "__bias__":
                _cmd = _cntrl[1]
                _core_id = _cmd[1]
                _attr_enum = getattr(bd.bdpars.BDHornEP, _cmd[2])
                if _core_id is not None:
                    _args = list([_core_id, _attr_enum])
                else:
                    _args = list([_attr_enum])
                for _idx in range(len(_cmd) - 3):
                    _args.append(_cmd[_idx + 3])
                _res = getattr(driver, _cmd[0])(*_args)
                if _res is None:
                    _res = ""
                cntrl_r.put(_res)

            # Check command queue
            try:
                _cmd = cmd_q.get(False, 1)
            except queue.Empty:
                continue

            try:
                #eval(compile(_cmd, '<string>', 'single'), g_dict, l_dict)
                _code = compile(_cmd, '<string>', 'single')
                eval(_code, {}, proc_dict)
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
        _last_time = driver.GetFPGATimeSec()
        spike_idx = np.zeros(4096, dtype=np.int64)
        spike_t = np.zeros(4096, dtype=np.float)

        while _do_run:
            spike_data = driver.RecvXYSpikesMasked(0)

            np.copyto(spike_idx, spike_data[0])
            np.copyto(spike_t, spike_data[1])

            _current_time = driver.GetFPGATimeSec()

            #for iy in range(16, 48):
            #    for ix in range(16, 48):
            #        ii = iy * 64 + ix
            #        spike_idx[ii] = 1
            #        spike_t[ii] = _current_time - 10e-9

            _delta_time = _current_time - _last_time
            _last_time = _current_time * 1.0

            #DECAY_AMOUNT = 1.0 / DECAY_PERIOD * REFRESH_PERIOD * REFRESH_PERIOD / _delta_time
            DECAY_AMOUNT = 1.0 / DECAY_PERIOD * _delta_time
            DECAY_MAT = np.full(4096, DECAY_AMOUNT, dtype=np.float32)
            _decay = MAX_MAT - DECAY_AMOUNT * (_current_time - spike_t) / _delta_time

            #_spiked = np.where(spike_idx == 1)[0]
            #if len(_spiked) > 0:
            #    print((_current_time, _delta_time, DECAY_AMOUNT, np.amax(spike_t[_spiked]), np.amin(spike_t[_spiked])))
            #    print(_decay[_spiked])

            #t_min = _cnt * POLL_PERIOD
            #t_max = t_min + POLL_PERIOD
            #spike_idx = np.zeros(4096, dtype=np.uint8)
            #spike_t = np.zeros(4096, dtype=np.float)
            #_rand = np.random.rand(4096)
            #_valid_idx = np.where(_rand > 0.9)[0]
            #spike_idx[_valid_idx] = 1
            #spike_t[_valid_idx] = _rand[_valid_idx] * POLL_PERIOD + t_min
            #_decay = MAX_MAT - DECAY_AMOUNT * (t_max - spike_t) / POLL_PERIOD

            np.subtract(ARR_DATA, DECAY_MAT, ARR_DATA)
            np.less(ARR_DATA, DECAY_MAT, ZERO_MASK)

            np.copyto(ARR_DATA, ZERO_MAT, where=ZERO_MASK)
            np.copyto(ARR_DATA, _decay, where=spike_idx.astype(bool))

            time.sleep(POLL_PERIOD)

    proc_dict = locals()

    cmd_thread = threading.Thread(target=__cmd_loop__)
    data_thread = threading.Thread(target=__data_loop__)

    cmd_thread.start()
    data_thread.start()

    cmd_thread.join()
    data_thread.join()

