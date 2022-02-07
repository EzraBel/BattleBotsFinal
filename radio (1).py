import time
import serial
import struct
import pygame
import pickle

def prep_lora():
    port = serial.Serial("/dev/ttyS0", baudrate=115200, timeout=3.0)
    print("Remote Active!")
    return port

def exit_lora(port, controller_props, log):
    print("Remote Deactivated!")
    log.close()
    pickle.dump(controller_props, open("controller.props", "wb"))
    port.close()
   
def prep_game():
    pygame.init()
    pygame.joystick.init()

    return pygame.joystick.Joystick(0)

# setting up functions

def process_macros(macro): #setting up kill sequences for the remote

    macro_dict = {}
    macro_dict['quit'] = [12,12,12]
    macro_dict['kill_radio'] = [3,3,3]
    macro_dict['enable_radio'] = [4,4,4]
    macro_dict['reset_props'] = [10,10,10]


    
    for key in macro_dict.keys():

        if(macro == macro_dict[key]):
            return key    

        macro_found = True

        for i in range(len(macro)):
            if(macro_dict[key][i] != -1 and macro_dict[key][i] != macro[i]):
                macro_found = False
            
        if macro_found:
            return key

    return ''
def get_controller_props():

    try:
        properties = pickle.load(open("controller.props", "rb"))
    except (OSError, IOError, EOFError) as e: # takes care of when there is an overwrite etc
        properties = {'sensitivity' : 1, 'trim' : 0, 'enabled' : True}
        pickle.dump(properties, open("controller.props", "wb"))

    return properties


#adjust values
def adjust_controller_props(props, trim_dir, sensitivity_dir):

    def trim_adj():
        return 0.05 * trim_dir

    def sens_adj():
        return 0.05 * sensitivity_dir

    props['sensitivity'] += sens_adj()
    props['trim'] += trim_adj()

    if props['sensitivity'] <= 0:
        props['sensitivity'] = 0.05
    
    if abs(props['trim']) > 1:
        props['trim'] = props['trim']/abs(props['trim'])
    
    return props
        

def send_data(data : struct, port : serial.Serial, refreshRate):
    port.write(data)
    time.sleep(refreshRate)


def main():

    # Initiate serial communication with LoRa device, obtain reference to device
    lora = prep_lora()

    # Initiate HID device and obtain reference to device. this is initializing the controller and game 
    controller = prep_game()
    controller.init()

    # Determine previous controller state (sensitivity, etc.)
    controller_props = get_controller_props()
    
    # Enable pygame ticks for HID values to be read
    clock = pygame.time.Clock()
    
    # Macros are 3 buttons kept running in order
    currentMacro = []


    
    while True:
    
        # PyGame event manager loop
        for event in pygame.event.get():

            if event.type == pygame.QUIT:
                exit_lora(lora, controller_props, log) #okay this makes sense. exit the lora instead of quitting the entire thing 

            elif event.type == pygame.JOYBUTTONDOWN:  

                # Keep track of the previous 3 buttons pressed  
                if(len(currentMacro) < 3): 
                    currentMacro.append(event.button + 1)
                else: 
                    currentMacro = currentMacro[1:]
                    currentMacro.append(event.button + 1) #same as my code except the +1

                    #Perform action based on value of currentMacro
                macro_result = process_macros(currentMacro)
                if macro_result == 'quit':
                    print("Quit \n\n\n")
                    exit_lora(lora, controller_props, log)
                    return
                elif macro_result == 'kill_radio':
                    print("Radio Killed \n\n\n")
                    controller_props['enabled'] = False

                elif macro_result == 'enable_radio':
                    print("Radio Enabled \n\n\n")
                    controller_props['enabled'] = True
                
                elif macro_result == 'reset_props':
                    print("Controller Properties Reset")
                    controller_props['enabled'] = True
                    controller_props['sensitivity'] = 1
                    controller_props['trim'] = 0

                #all the same just more concise. referencing predefined
                #just need to quit when nothing sent 
            
            elif event.type == pygame.JOYHATMOTION:

                # Obtain Hat values and adjust trim and sensitivity values
                trim_ind, sens_ind = event.value

                controller_props = adjust_controller_props(controller_props, trim_ind, sens_ind)
    
        # Obtain axis values
        ax_vals = []
        for i in range(controller.get_numaxes()): 
            axis = controller.get_axis(i)
            ax_vals += [axis] 

        
        
        data = struct.pack("<ffff", ax_vals[0], ax_vals[1], ax_vals[2], ax_vals[3])
        props = struct.pack("<ff", float(controller_props['trim']), float(controller_props['sensitivity']))
        
        print("Axes: <", end = '')
        for i in range(len(ax_vals)):   
            print("{} {:>6.3f} >|<".format(i, ax_vals[i]), end = '')
        
        print(">==<Sensitivity: {:>6.3f}>==<Trim: {:>6.3f}>==<Macro: {}>".format(controller_props['sensitivity'], controller_props['trim'], str(currentMacro)), end = '\r')
        
        if controller_props['enabled']:
            send_data(data, lora, 1.0/30)
            send_data(props, lora, 1.0/30)
            #send(props, lora)
            
            log = open("log.txt", "w+")
            log.writelines(str(data))
        
        pickle.dump(controller_props, open("controller.props", "wb"))



if __name__ == "__main__":
    main()
