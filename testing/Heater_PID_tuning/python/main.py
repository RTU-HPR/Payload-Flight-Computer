import socket
import queue
import threading
import random
from time import sleep
import datetime
import dearpygui.dearpygui as dpg

# Get local IP address
from socket import gethostname, gethostbyname
HOSTNAME = gethostname()
LOCAL_IP_ADDRESS = gethostbyname(HOSTNAME)

TM_ADDRESS = (str(LOCAL_IP_ADDRESS), 10035)
TC_ADDRESS = ('192.168.236.143', 10045)

# Sockets
tm_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
tc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
tm_socket.bind(TM_ADDRESS)

# Queues
received_messages = queue.Queue()
sendable_messages = queue.Queue()

# Data lists
Time = []
STS35_temperature = []
BMP180_temperature = []
Target_temperature = []
Current_target_temperature = []
BMP180_pressure = []
Proportional = []
Integral = []
Derivative = []
PWM_Output = []


def send_to_payload(): 
  while True:   
    message = sendable_messages.get()
    tc_socket.sendto(message, TC_ADDRESS)
    sendable_messages.task_done()

def receive_from_payload():
  # Receive a message from the transceiver
  # message, addr = tm_socket.recvfrom(4096)    
  # message = message.decode()
  # Get current time
  start_time = datetime.datetime.now()
  while True:
    time = (datetime.datetime.now() - start_time).total_seconds()
    
    message = f"{time},"
    for i in range(10):
      message += "{:.2f},".format(random.uniform(0, 100))
    # Remove last comma
    message = message[:-1]
    
    received_messages.put(message)
    
    sleep(0.1)
    

def parse_received():
  while True:
    if not received_messages.empty():
      message = received_messages.get()
      message = [float(x) for x in message.split(',')]

      Time.append(message[0])
      STS35_temperature.append(message[1])
      BMP180_temperature.append(message[2])
      Target_temperature.append(message[3])
      Current_target_temperature.append(message[4])
      BMP180_pressure.append(message[5])
      Proportional.append(message[6])
      Integral.append(message[7])
      Derivative.append(message[8])
      PWM_Output.append(message[9])
      
      dpg.set_value("sts_temperature", [list(Time), list(STS35_temperature)])
      dpg.set_value("bmp_temperature", [Time, BMP180_temperature])
      dpg.set_value("target_temperature", [Time, Target_temperature])
      dpg.set_value("current_target_temperature", [Time, Current_target_temperature])
      dpg.set_value("pressure", [Time, BMP180_pressure])
      dpg.set_value("proportional", [Time, Proportional])
      dpg.set_value("integral", [Time, Integral])
      dpg.set_value("derivative", [Time, Derivative])
      dpg.set_value("pwm_output", [Time, PWM_Output])
      
      dpg.fit_axis_data("temp_x_axis")
      dpg.fit_axis_data("pressure_x_axis")
      dpg.fit_axis_data("pid_x_axis")
      dpg.fit_axis_data("temp_y_axis")
      dpg.fit_axis_data("pressure_y_axis")
      dpg.fit_axis_data("pid_y_axis")

      received_messages.task_done()
    sleep(0.01)

if __name__ == "__main__":
  dpg.create_context()
    
  with dpg.window(tag="Primary Window"):    
    with dpg.plot(label="Temperature plots", height=300, width=-1):
      dpg.add_plot_legend()
      dpg.add_plot_axis(dpg.mvXAxis, label="Time (s)", tag="temp_x_axis")
      dpg.add_plot_axis(dpg.mvYAxis, label="Temperature (C)", tag="temp_y_axis")

      dpg.add_line_series(x=[0], y=[0], label="STS35 temperature", parent="temp_y_axis", tag="sts_temperature")
      dpg.add_line_series(x=[0], y=[0], label="BMP180 temperature", parent="temp_y_axis", tag="bmp_temperature")
      dpg.add_line_series(x=[0], y=[0], label="Target temperature", parent="temp_y_axis", tag="target_temperature")
      dpg.add_line_series(x=[0], y=[0], label="Current target temperature", parent="temp_y_axis", tag="current_target_temperature")
    
    with dpg.plot(label="Pressure plots", height=300, width=-1):
      dpg.add_plot_legend()
      dpg.add_plot_axis(dpg.mvXAxis, label="Time (s)", tag="pressure_x_axis")
      dpg.add_plot_axis(dpg.mvYAxis, label="Pressure (Pa)", tag="pressure_y_axis")

      dpg.add_line_series(x=[0], y=[0], label="BMP180 pressure", parent="pressure_y_axis", tag="pressure")

    with dpg.plot(label="PID plots", height=300, width=-1):
      dpg.add_plot_legend()
      dpg.add_plot_axis(dpg.mvXAxis, label="Time (s)", tag="pid_x_axis")
      dpg.add_plot_axis(dpg.mvYAxis, label="Values", tag="pid_y_axis")

      dpg.add_line_series(x=[0], y=[0], label="Proportional", parent="pid_y_axis", tag="proportional")
      dpg.add_line_series(x=[0], y=[0], label="Integral", parent="pid_y_axis", tag="integral")
      dpg.add_line_series(x=[0], y=[0], label="Derevative", parent="pid_y_axis", tag="derivative")
      dpg.add_line_series(x=[0], y=[0], label="PWM Output", parent="pid_y_axis", tag="pwm_output")

      
  # Start threads
  receive_thread = threading.Thread(target=receive_from_payload)
  receive_thread.daemon = True
  receive_thread.start()
  
  send_thread = threading.Thread(target=send_to_payload)
  send_thread.daemon = True
  send_thread.start()

  parsing_thread = threading.Thread(target=parse_received)
  parsing_thread.daemon = True
  parsing_thread.start()

  dpg.create_viewport(title='PID Heating System Monitor')
  dpg.setup_dearpygui()
  dpg.show_viewport()
  dpg.set_primary_window("Primary Window", True)
  dpg.maximize_viewport()
  dpg.start_dearpygui()
  dpg.destroy_context()