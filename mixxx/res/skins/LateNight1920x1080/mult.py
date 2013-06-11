#!/usr/bin/python

import os, sys, os.path

for fname in sys.argv[1:]:
    f = open(fname)
    
    for line in f.readlines():
        line = line.rstrip()
        if "<POS>" in line.upper():
            s = line.upper().find("<POS>") + 5
            e = line.upper().find("</POS")
            if e == s:
                print line
                continue
            numbers = line[s:e].split(',')
            new_numbers = (str(int(round(int(numbers[0])*1.5))), str(int(round(int(numbers[1])*1.5))))
            print line[0:s] + ','.join(new_numbers) + line[e:]
        elif "<SIZE>" in line.upper():
            s = line.upper().find("<SIZE>") + 6
            e = line.upper().find("</SIZE")
            if e == s or e == -1:
                print line
                continue
            i = s
            in_number = False
            num = ""
            value = []
            while i < e:
                if line[i] == ',':
                    if in_number:
                        num = str(int(round(int(num)*1.5)))
                    value.append(num)
                    value.append(',')
                    in_number = False
                    num = ""
                elif line[i] in ('1','2','3','4','5','6','7','8','9','0'):
                    if not in_number:
                        value.append(num)
                        num = ""
                    in_number = True
                    num += line[i]
                else:
                    if in_number:
                        num = str(int(round(int(num)*1.5)))
                        value.append(num)
                        num = ""
                    in_number = False
                    num += line[i]
                i+=1
            if in_number:
                num = str(int(round(int(num)*1.5)))
            value.append(num)
            #print "WHAT",line, value
            print line[0:s] + ''.join(value) + line[e:] 
        elif "LUCIDA" in line.upper():
            e = line.upper().find(" LUCIDA")
            s = e - 1
            while line[s] != ' ':
                s -= 1
            s += 1
            i = s
            in_number = False
            num = ""
            value = []
            while i < e:
                if line[i] == ',':
                    if in_number:
                        num = str(int(round(int(num)*1.25)))
                    value.append(num)
                    value.append(',')
                    in_number = False
                    num = ""
                elif line[i] in ('1','2','3','4','5','6','7','8','9','0'):
                    if not in_number:
                        value.append(num)
                        num = ""
                    in_number = True
                    num += line[i]
                else:
                    if in_number:
                        num = str(int(round(int(num)*1.25)))
                        value.append(num)
                        num = ""
                    in_number = False
                    num += line[i]
                i+=1
            if in_number:
                num = str(int(round(int(num)*1.25)))
            value.append(num)
            print line[0:s] + ''.join(value) + line[e:]
                
        else:
            print line   
    

