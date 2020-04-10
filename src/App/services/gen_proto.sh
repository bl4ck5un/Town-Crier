OUTDIR=./generated

mkdir -p ./generated
protoc --grpc_out=$OUTDIR \
    --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` \
    rpc.proto

protoc --cpp_out=$OUTDIR tc.proto