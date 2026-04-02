#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
# "app" for testing Experior (the microservice intended for experimental Python code)

import grpc
from concurrent import futures
import microservice_pb2
import microservice_pb2_grpc
import time

class MakeComparisonsServicer(microservice_pb2_grpc.MakeComparisonsServicer):
    
    def Is_x_GT_y(self, request, context):
        response = microservice_pb2.MessageTypeB()
        response.reply = request.x > request.y
        return response

    def Is_y_LT_z(self, request, context):
        response = microservice_pb2.MessageTypeB()
        response.reply = request.y < request.z
        return response

#VVVVVVVV1VVVVVVVVV2VVVVVVVVV3VVVVVVVVV4VVVVVVVVV5VVVVVVVVV6VVVVVVVVV7VVVVVVVVV8VVVVVVVVV9VVVVVVVVVCVVVV5
#def main():
def serve():
    
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    microservice_pb2_grpc.add_MakeComparisonsServicer_to_server(MakeComparisonsServicer(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    print("Server started. Listening on port 50051.")
    server.wait_for_termination()

if __name__ == '__main__':
# above TRUE if this file run from a shell, Docker RUN (in build), or Docker CMD/ENTRYPOINT (at runtime)
# [i.e., exempts file from default Py top-to-bottom exec, instead giving it an entry point and exec order
# Otherwise, FALSE, as __name__ is set to the filename (w/ no *.py), e.g., when imported as a module.]
        serve()

#EOF
#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXX5
