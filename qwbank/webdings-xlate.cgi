#! /usr/bin/env python
# -*- coding: utf-8 -*-

# 20201127/http://www.alanwood.net/demos/webdings.html

# Python 3 style print to print without newline.
from __future__ import print_function

import cgitb
import cgi

xlate_str = "          \n  \r                   🕷🕸🕲🕶🏆🎖🖇🗨🗩🗰🗱🌶🎗🙾🙼🗕🗖🗗⏴⏵⏶⏷⏪⏩⏮⏭⏸⏹⏺🗚🗳🛠🏗🏘🏙🏚🏜🏭🏛🏠🏖🏝🛣🔍🏔👁👂🏞🏕🛤🏟🛳🕬🕫🕨🔈🎔🎕🗬🙽🗭🗪🗫⮔✔🚲□🛡📦🛱■🚑🛈🛩🛰🟈🕴⚫🛥🚔🗘🗙❓🛲🚇🚍⛳🛇⊖🚭🗮|🗯🗲 🚹🚺🛉🛊🚼👽🏋⛷🏂🏌🏊🏄🏍🏎🚘🗠🛢💰🏷💳👪🗡🗢🗣✯🖄🖅🖃🖆🖹🖺🖻🕵🕰🖽🖾📋🗒🗓📖📚🗞🗟🗃🗂🖼🎭🎜🎘🎙🎧💿🎞📷🎟🎬📽📹📾📻🎚🎛📺💻🖥🖦🖧🕹🎮🕻🕼📟🖁🖀🖨🖩🖿🖪🗜🔒🔓🗝📥📤🕳🌣🌤🌥🌦☁🌧🌨🌩🌪🌬🌫🌜🌡🛋🛏🍽🍸🛎🛍Ⓟ♿🛆🖈🎓🗤🗥🗦🗧🛪🐿🐦🐟🐕🐈🙬🙮🙭🙯🗺🌍🌏🌎🕊"

def old2asc(user):
    for c in user:
        # N.B. Python 3 requires `chr()` before printing.
        print(c.encode('utf-16')[2], end="")

    print("")

def asc2new(source):
    user = source.encode('ascii')

    for c in user:
        # N.B. Python 3 does not require `ord()` and `decode()` here.
        # N.B. For Python 2 CGI we need to do one last encode for
        # printing the single character as just a string of bytes.
        idx = ord(c)
        print(xlate_str.decode('utf-8')[idx].encode('utf-8'), end="")

    print("")

def old2new(user):
    for c in user:
        # N.B. Python 3 does not require `ord()` and `decode()` here.
        # N.B. For Python 2 CGI we need to do one last encode for
        # printing the single character as just a string of bytes.
        idx = ord(c.encode('utf-16')[2])
        print(xlate_str.decode('utf-8')[idx].encode('utf-8'), end="")

    print("")

def cli():
    print("Choose your function:")
    print("0 == old Webdings private-use Unicode to ASCII")
    print("1 == ASCII to new Unicode-equivalent Webdings")
    print("2 == old Webdings private-use Unicode to new Unicode Webdings")
    # N.B. Use just `input()` in Python 3.
    choice = int(raw_input("Choice? "))
    if choice < 0 or choice > 2:
        print("Invalid choice.")

    else:
        message = raw_input("Type your message:\n")

        if choice == 0:
            old2asc(message)
        elif choice == 1:
            asc2new(message)
        elif choice == 2:
            old2new(message)

def docgi():
    # Enables special webpage display of exception handler
    cgitb.enable()

    print("Content-Type: text/plain; charset=utf-8")
    print()

    form = cgi.FieldStorage()
    if "message" not in form or "cnvtype" not in form:
        print("Error!  No conversion type and message filled out.")
        return
    choice = int(form["cnvtype"].value)
    message = form["message"].value
    if choice == 0:
        old2asc(message)
    elif choice == 1:
        asc2new(message)
    elif choice == 2:
        old2new(message)
    else:
        print("Invalid choice:", choice)
        return

docgi()
#cli()
