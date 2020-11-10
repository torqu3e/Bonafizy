#!/usr/bin/env python3

import json
import pytest
import requests
import time

FW_VER = "0.1.5"

API_ENDPOINTS = [
    "power/off",
    "hold/off",
    "hold/on",
    "power/on",
    "hold/off",
    "hold/on",
    "hold/on",
    "power/off",
    "brew",
    "brew",
    "power/off",
    "state",
    "coffee",
]
RESPONSES = [
    '{"message": "Kettle already off", "state": {"power": 0, "hold": 0}}',
    '{"message": "Kettle is off, cannot turn hold off", "state": {"power": 0, "hold": 0}}',
    '{"message": "Kettle is off, cannot turn hold on", "state": {"power": 0, "hold": 0}}',
    '{"message": "Kettle powered on", "state": {"power": 1, "hold": 0}}',
    '{"message": "Hold already off", "state": {"power": 1, "hold": 0}}',
    '{"message": "Hold temperature on", "state": {"power": 1, "hold": 1}}',
    '{"message": "Hold temperature already on", "state": {"power": 1, "hold": 1}}',
    '{"message": "Kettle powered off", "state": {"power": 0, "hold": 0}}',
    '{"message": "Brewing now!!!", "state": {"power": 1, "hold": 1}}',
    '{"message": "Kettle already on", "state": {"power": 1, "hold": 1}}',
    '{"message": "Kettle powered off", "state": {"power": 0, "hold": 0}}',
    f'{{"version": "{FW_VER}", "state": {{"power": 0, "hold": 0}}}}',
    '<html><h3><a href="https://tools.ietf.org/html/rfc2324#section-2.3.2">I\'m a Teapot</a></h2></html>',
]


def check_response(endpoint):
    r = requests.get(f"http://192.168.1.186/{endpoint}")
    time.sleep(1)
    return r.text


def test_bonafizy():
    for i in range(len(API_ENDPOINTS) - 1):
        assert json.loads(check_response(API_ENDPOINTS[i])) == json.loads(RESPONSES[i])


def test_httpcp():
    assert check_response(API_ENDPOINTS[-1]) == RESPONSES[-1]
