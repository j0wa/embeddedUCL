from __future__ import print_function, unicode_literals

from PyInquirer import style_from_dict, Token, prompt, Separator
from pprint import pprint


style = style_from_dict({
    Token.Separator: '#cc5454',
    Token.QuestionMark: '#673ab7 bold',
    Token.Selected: '#cc5454',  # default
    Token.Pointer: '#673ab7 bold',
    Token.Instruction: '',  # default
    Token.Answer: '#f44336 bold',
    Token.Question: '',
})

#Here is the possible thing we can do to our devices in the network.
#The name of the choices are then converted into function in python. e.g. line 58
questions = [
    {
        'type': 'checkbox',
        'message': 'Select option',
        'name': 'option',
        'choices': [
            Separator('= Temperature Captors ='),
            {
                'name': 'temp1'
            },
            {
                'name': 'temp2'
            },
            Separator('= Thermostat Controller  ='),
            {
                'name': 'thermo1',
                'checked': True
            },
            {
                'name': 'thermo2'
            },
            {
                'name': 'thermo3'
            },
            Separator('= Light  ='),
            {
                'name' : 'turn_on_or_off_the_lamp',
                'message': 'Turn_on_or_off_the_lamp'
            },
            Separator('= Wall Switch  ='),
            {
                'name' : 'wall_switch',
            },
            Separator('= Movement Sensor  ='),
            {
                'name' : 'toggle_sensor',
            }
        ],
        'validate': lambda answer: 'You must choose at least one option.' \
            if len(answer) == 0 else True
    }
]
#Toggle sensor from line 50
def wall_switch():
    print("wall switch")
def toggle_sensor():
    #Call the sensor
    print("toggle sensor")
    
def turn_on_or_off_the_lamp():
    print("turn on / off the lamp")
def thermo1():
    print("pong")
def answer(answers):
    print(answers)
    for value in answers.values():
        for ans in value:
            eval(ans+"()")

def main():
    print("Hey!\n Welcome to our interface!\n Here, you can decide which interaction you want with our network")
    print("Created by: Lechantre Adrien\t Wathelet Jolan \t and Van Erps Malicia")

    answers = prompt(questions, style=style)
    print("You choose:")
    answer(answers)
main()
 