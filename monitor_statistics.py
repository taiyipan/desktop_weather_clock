from time import sleep
from datetime import datetime, timedelta
import requests
import json
from pySerialTransfer import pySerialTransfer as txfer

# api parameters
latitude = 0
longitude = 0
api_key = ''
units = 'metric'
url = 'https://api.openweathermap.org/data/2.5/weather?lat={}&lon={}&appid={}&units={}'.format(latitude, longitude, api_key, units)
# global variable
last_api_call = None
last_package = 0
network_connection = True

# validate api call timing, limit call frequency to avoid api lockdown
def validate_api_call() -> bool:
    global last_api_call
    # if first loop or if 5 minutes had elapsed since last API call
    if last_api_call is None or datetime.now() > last_api_call + timedelta(minutes = 5):
        last_api_call = datetime.now()
        return True
    else:
        return False

# call weather api and return json data
def call_weather_api():
    global network_connection
    if validate_api_call():
        try:
            response = requests.get(url, timeout = 5)
            print('status code: {}'.format(response.status_code))
            network_connection = True
            if response.status_code == 200:
                return response.json()
        except:
            network_connection = False
            print('Network connection error', datetime.now())
            return -1 # network connection error
    if network_connection:
        return None # skip weather API call, display time as usual
    else:
        return -1

# build data package as numerical value
def build_data_package():
    global last_package
    # api call
    data = call_weather_api()
    if data is None:
        return (last_package // 10**7 % 10) * 10**7 + (last_package // 10**6 % 10) * 10**6 + datetime.now().hour * 10**4 + datetime.now().minute * 10**2 + (last_package // 10 % 10) * 10 + (last_package % 10)
    elif data == -1:
        package = -88888801
        return package

    # process data
    temp = data['main']['temp']
    weather = data['weather'][0]['id']
    sunrise = datetime.fromtimestamp(data['sys']['sunrise'])
    sunset = datetime.fromtimestamp(data['sys']['sunset'])
    negative = temp < 0
    temp = int(round(abs(temp)))

    # build package value
    package = 0
    if weather < 800:
        package += 1
    if datetime.now() > sunrise and datetime.now() < sunset:
        package += 10
    package += datetime.now().hour * 10**4 + datetime.now().minute * 10**2
    package += temp * 10**6
    if negative:
        package *= -1
    # update last_package
    last_package = package
    # return package
    print('package value: {}, timestamp: {}'.format(package, datetime.now()))
    return package

# program call
if __name__ == '__main__':
    link = txfer.SerialTransfer('/dev/ttyACM0')
    link.open()
    sleep(5)
    while True:
        package = build_data_package()
        # print(package)
        sendSize = link.tx_obj(package)
        link.send(sendSize)
        sleep(3)
    link.close()
