// Manual packing test functions

var packet = new HIDPacket('test', [0x1, 0x2], 6)
packet.addOutput('test', 'ushort', 2, 'H')
packet.addOutput('test', 'short', 4, 'h')

var field = packet.getField('test', 'ushort')
print('FIELD ' + field.id + ' MIN ' + field.min + ' MAX ' + field.max)

field.value = 1024
field = packet.getField('test', 'short')
field.value = -32767
print('FIELD ' + field.id + ' MIN ' + field.min + ' MAX ' + field.max)

var out = { 'length': packet.length, 'data': [] }
for (var i = 0; i < packet.header.length; i++) {
    out.data[i] = i
}
for (var group_name in packet.groups) {
    var group = packet.groups[group_name]
    for (var field_name in group) {
        var field = group[field_name]
        print('PACKING ' + field.id)
        packet.pack(out, field)
    }
}
for (var i = 0; i < out.length; i++) {
    print('BYTE ' + i + ' VALUE ' + out.data[i])
}
