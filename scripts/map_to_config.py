#!/usr/bin/env python3
"""
K. Miernik
2013
Generate basic <map> section for SHE DSSD for the new pixie_ldf_c Config.xml file
based on old style map.txt

"""
import xml.dom.minidom

def test_load(data):
    """Test how the data are loaded from map.txt file"""
    for module in data:
        print('Module: ', module['number'])
        for channel in module['channels']:
            print('     Ch: {} Loc: {} T: {} S: {}'.format(channel['number'],
                                                    channel['location'],
                                                    channel['type'],
                                                    channel['subtype']))
            print('         cal: {}'.format(channel['calibration']))


if __name__ == '__main__':
    data = [] 

    with open("map.txt") as map_file:
        lineN = 0
        module = None
        for line in map_file:
            lineN += 1
            line = line.strip()
            if line.startswith('%') or len(line) < 1:
                continue
            fields = line.split() 
            mod = fields[0]
            ch = fields[1]
            # Ignore raw channel number at [2]
            det_type = fields[3]
            det_subtype = fields[4]
            loc = fields[5]
            channel = {'number': ch, 
                    'location' : loc, 
                    'type' : det_type,
                    'subtype' : det_subtype}
            if module is None:
                module = {'number' : mod, 'channels' : []}
            elif module['number'] != mod:
                data.append(module)
                module = {'number' : mod, 'channels' : []}
            module['channels'].append(channel)


    with open("cal.txt") as cal_file:
        lineN = 0
        for line in cal_file:
            lineN += 1
            line = line.strip()
            if line.startswith('%') or len(line) < 1:
                continue
            fields = line.split() 
            loc = fields[0]
            det_type = fields[1]
            det_subtype = fields[2]
            # Ignore number of calibrations at [3], order at [4],
            # and threshold at [5]
            # Notice that it will get only the linear factors for the 
            # first calibration
            a0 = float(fields[6])
            a1 = float(fields[7])
            found = False
            for module in data:
                for channel in module['channels']:
                    if (channel['type'] == det_type and
                        channel['subtype'] == det_subtype and
                        channel['location'] == loc):
                        channel.update({'calibration' : [a0, a1]})
                        found = True
                if found:
                    break
            else:
                print('Could not find channel: type {} subtype {} loc {}'.
                      format(det_type, det_subtype, loc))

    #test_load(data)

    out = open('out.xml', 'w')
    dom = xml.dom.minidom.getDOMImplementation()
    table = dom.createDocument(None, "Map", None)
    root = table.documentElement
    
    for m in data:
        module = table.createElement("Module")
        module.setAttribute("number", m['number'])
        for c in m['channels']:
            channel = table.createElement("Channel")
            # Force attriute number to be first (they are sorted alphabetically)
            channel.setAttribute("aa_number", c['number'])
            channel.setAttribute("type", c['type'])
            channel.setAttribute("subtype", c['subtype'])
            channel.setAttribute("location", c['location'])
            calibration = table.createElement("Calibration")
            calibration.setAttribute("model", "linear")
            calibration.setAttribute("max", str(32000))
            text_element = table.createTextNode('{} {}'.
                    format(c['calibration'][0], c['calibration'][1]))
            calibration.appendChild(text_element)
            channel.appendChild(calibration)
            module.appendChild(channel)
        root.appendChild(module)
    
    out.write(
        table.toprettyxml(indent="    ", encoding="utf-8").decode("utf-8"))
