# microPython implementation for a wifi stack on a Bonavita kettles

import machine
from machine import Pin
import network
import utime
from microWebSrv import MicroWebSrv


WIFI_SSID = 'dirty'  # 2.4 GHz only
WIFI_PASSWORD = 'bigboobies'

POWER_BUTTON = D1
HOLD_BUTTON = D2
POWER_LED = D3
HOLD_LED = D4


def setup():
    machine.freq(80000000)
    power_button = Pin(POWER_BUTTON, Pin.OUT)
    hold_button = Pin(HOLD_BUTTON, Pin.OUT)
    power_led = Pin(POWER_LED, Pin.IN)
    hold_led = Pin(HOLD_LED, Pin.IN)


def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('connecting to network...')
        wlan.connect(WIFI_SSID, WIFI_PASSWORD)
        while not wlan.isconnected():
            pass
    print('network config:', wlan.ifconfig())


def power_on():
    if not power_led.value():
        power_button.on()
        utime.sleep_ms(100)
        power_button.off()


def power_off():
    if power_led.value():
        power_button.on()
        time.sleep_ms(100)
        power_button.off()


def hold_on():
    if not hold_led.value():
        hold_button.on()
        utime.sleep_ms(100)
        hold_button.off()


def hold_off():
    if hold_led.value():
        hold_button.on()
        utime.sleep_ms(100)
        hold_button.off()
        


mws = MicroWebSrv()      # TCP port 80 and files in /flash/www
mws.Start()


@MicroWebSrv.route('/state')
def get_state():
    print(f"power : {power_led.value()}, hold : {hold_led.value()}")

@MicroWebSrv.route('/state/power')
def get_state_power():
    print(f"power : {power_led.value()}")

@MicroWebSrv.route('/state/hold')
def get_state_hold():
    print(f"hold : {hold_led.value()}")

@MicroWebSrv.route('/state/power/on', 'POST')
def switch_power_on():
    power_on()

@MicroWebSrv.route('/state/power/off', 'POST')
def switch_power_off():
    power_off()

@MicroWebSrv.route('/state/hold/on', 'POST')
def switch_hold_on():
    hold_on()

@MicroWebSrv.route('/state/hold/off', 'POST')
def switch_hold_off():
    hold_off()