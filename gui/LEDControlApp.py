import kivy
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from azure.iot.hub import IoTHubRegistryManager

count = 0
CONNECTION_STRING = "HostName=ecen5713-iot-hub.azure-devices.net;SharedAccessKeyName=service;SharedAccessKey=Gply81vOUaYpgBujYD0xhIk13xzExrKSf0EDvTW0LDA="
DEVICE_ID = "ecen5713-iot-edge"
def iothub_messaging_sample_run(data):
    global count
    registry_manager = IoTHubRegistryManager(CONNECTION_STRING)
    count+=1
    print ( 'Sending message: {0}'.format(count) )
    print('Data send: {0}'.format(data))
    props={}
    registry_manager.send_c2d_message(DEVICE_ID, data, properties=props)

class MyGrid(GridLayout):
    def __init__(self, **kwargs):
        super(MyGrid, self).__init__(**kwargs)
        self.cols = 1

        self.inside = GridLayout()
        self.inside.cols = 2

        self.inside.add_widget(Label(text="LED Command: "))
        self.cmd = TextInput(multiline=False)
        self.inside.add_widget(self.cmd)

        self.add_widget(self.inside)

        self.Send = Button(text="Send", font_size=80)
        self.Send.bind(on_press=self.pressed)
        self.add_widget(self.Send)

    def pressed(self, instance):
        cmd = self.cmd.text
        iothub_messaging_sample_run(cmd)
        #print("receive:", cmd)
        self.cmd.text = ""

class LEDControlApp(App):
    def build(self):
        return MyGrid()


if __name__ == "__main__":
    LEDControlApp().run()