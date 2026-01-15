#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX
# "app" for tests of Slate

import requests
import time

def main():
    while True:
        try:
            response = requests.get('http://rest:9876/api/')
            print(response.text)
        except requests.exceptions.RequestException as e:
            print(f"Request failed: {e}")
        time.sleep(5)

if __name__ == "__main__":
    main()
