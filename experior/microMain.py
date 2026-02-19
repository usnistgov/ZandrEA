#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX
# "app" for testing Experior (the microservice intended for experimental Python code)

from concurrent import futures
#import logging

#import grpc
#
#import microservice_pb2
#import microservice_pb2_grpc
#
#class MakeComparison(microservice_pb2_grpc.MakeComparisonServicer):
#    def Is_x_bigger(self, input, context):
#        reply = input.x > input.y
#        return microservice_pb2.ComparisonReply(reply=reply)
#
#def main():
#    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
#    microservice_pb2_grpc.add_MakeComparisonServicer_to_server(MakeComparison(), server)
#    server.add_insecure_port('[::]:50051')
#    server.start()
#    print("gRPC server started on port 50051.")
#    server.wait_for_termination()

def main():
 
    #The main function to test execution.

    print("Python main() success!")
    # Calling further Python functions
    run_test_logic()
    
def run_test_logic():

    # A helper function called from main().
    print("Helper function success.")

if __name__ == '__main__':
# above TRUE if this file run from a shell, Docker RUN (in build), or Docker CMD/ENTRYPOINT (at runtime)
# [i.e., exempts file from default Py top-to-bottom exec, instead giving it an entry point and exec order
# Otherwise, FALSE, as __name__ is set to the filename (w/ no *.py), e.g., when imported as a module.]
#   logging.basicConfig()
    main()
