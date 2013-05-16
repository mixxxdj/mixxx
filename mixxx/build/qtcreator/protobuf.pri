#
# Qt qmake integration with Google Protocol Buffers compiler protoc
#
# To compile protocol buffers with qt qmake, specify PROTOS variable and
# include this file
#
# Example:
# LIBS += /usr/local/lib/libprotobuf.so
# PROTOS = a.proto b.proto
# include(protobuf.pri)
#
# By default protoc looks for .proto files (including the imported ones) in
# the current directory where protoc is run. If you need to include additional
# paths specify the PROTOPATH variable
#
# Found at http://www.kieltech.de/uweswiki/Google%20Protocol%20Buffers
#

PROTOPATH += .
#PROTOPATH += $$BASE_DIR/src/proto
PROTOPATH += ..\mixxx-winsrc\src\proto
PROTOPATHS =
for(p, PROTOPATH):PROTOPATHS += --proto_path=$${p}

protobuf_decl.name = protobuf header
protobuf_decl.input = PROTOS
protobuf_decl.output = ${QMAKE_FILE_BASE}.pb.h
protobuf_decl.commands = protoc --cpp_out="." $${PROTOPATHS} ${QMAKE_FILE_NAME}
protobuf_decl.variable_out = GENERATED_FILES
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf implementation
protobuf_impl.input = PROTOS
protobuf_impl.output = ${QMAKE_FILE_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\n)
protobuf_impl.variable_out = GENERATED_SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl

