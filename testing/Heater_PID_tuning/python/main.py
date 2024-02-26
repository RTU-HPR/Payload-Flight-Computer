import socket
import queue
import threading
import random
from time import sleep
import datetime
import dearpygui.dearpygui as dpg 
import os

# Get local IP address
from socket import gethostname, gethostbyname
HOSTNAME = gethostname()
LOCAL_IP_ADDRESS = gethostbyname(HOSTNAME)

TM_ADDRESS = (str(LOCAL_IP_ADDRESS), 10035)
TC_ADDRESS = ('192.168.136.205', 10045)

# Sockets
tm_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
tc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
tm_socket.bind(TM_ADDRESS)

# Queues
received_messages = queue.Queue()
sendable_messages = queue.Queue()

# Data lists
Time = []
Outside_temperature = []
STS35_temperature = []
BMP180_temperature = []
Target_temperature = []
Current_target_temperature = []
BMP180_pressure = []
Proportional = []
Integral = []
Derivative = []
PWM_Output = []

time_offset = 0

def send_to_payload(): 
  while True:   
    message = "hello".encode()#sendable_messages.get()
    tc_socket.sendto(message, TC_ADDRESS)
    # sendable_messages.task_done()
    sleep(5)

def receive_from_payload():
  while True:
    # Receive a message from the transceiver
    message, addr = tm_socket.recvfrom(4096)    
    message = message.decode()    
    received_messages.put(message)    

def parse_received():
  # Create a new csv log file and add the header. The file will be named with the current date and time
  file_name = f"logs/{datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}.csv"
  log_file = open(file_name, "w")
  log_file.write("Time,STS35 temperature,BMP180 temperature,Target temperature,Current target temperature,BMP180 pressure,Proportional,Integral,Derivative,PWM Output,Outside temperature\n")
  log_file.close()
  
  message_count = 0
  
  while True:
    if not received_messages.empty():
      message_count += 1
      message = received_messages.get()
      message = [float(x) for x in message.split(',')]
      
      if len(Time) == 0:
        time_offset = message[0] / 1000
        
      # Print the values with names
      os.system('cls')
      print(f"Message count: {message_count}")
      print(f"Time: {round(message[0] / 1000, 1)} | Time running: {round(message[0] / 1000 - time_offset, 1)} seconds")
      print(f"Container temperature - STS35: {message[1]} °C | BMP180 temperature: {message[2]} °C")
      print(f"Target temperature: {message[3]} °C | Current target temperature: {message[4]} °C")
      print(f"Outside temperature: {message[10]} °C")
      print(f"BMP180 pressure: {int(message[5])} Pa")
      print(f"Battery voltage: {message[11]} V")
      print(f"P: {message[6]} | I: {message[7]} | D: {message[8]} | PWM: {message[9]} | Actual PWM: {int(message[9])}")
        
      Time.append((message[0] / 1000) - time_offset)
      STS35_temperature.append(message[1])
      BMP180_temperature.append(message[2])
      Target_temperature.append(message[3])
      Current_target_temperature.append(message[4])
      BMP180_pressure.append(message[5])
      Proportional.append(message[6])
      Integral.append(message[7])
      Derivative.append(message[8])
      PWM_Output.append(message[9])
      Outside_temperature.append(message[10])
      
      if message_count % 10 == 0:
        with open(file_name, "a") as log_file:
          log_file.write(f"{round(message[0] / 1000 - time_offset, 1)},{message[1]},{message[2]},{message[3]},{message[4]},{message[5]},{message[6]},{message[7]},{message[8]},{message[9]},{message[10]}\n")
      
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