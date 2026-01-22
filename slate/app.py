#XXXXXXXX1XXXXXXXXX2XXXXXXXXX3XXXXXXXXX4XXXXXXXXX5XXXXXXXXX6XXXXXXXXX7XXXXXXXXX8XXXXXXXXX9XXXXXXXXXCXXXXX
# "app" for tests of Slate

import requests
import time

def main():
    headers = {'Content-Type': 'application/json'}
    while True:
        try:
            response = requests.get('http://0.0.0.0:9876/subjects', headers=headers)
            subjects = response.json().get("subjects")
            #print(subjects)
            print("Hello, Dan")
        except requests.exceptions.RequestException as e:
            print(f"Request failed: {e}")
        time.sleep(5)

if __name__ == "__main__":
    main()
