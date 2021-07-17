import sys
import requests

def classify(key, numbers):
    url = "https://machinelearningforkids.co.uk/api/scratch/"+ key + "/classify"

    response = requests.post(url, json={ "data" : numbers })

    if response.ok:
        responseData = response.json()
        topMatch = responseData[0]
        return topMatch
    else:
        response.raise_for_status()

def main(argv):
    atk = int(argv[1])
    defe = int(argv[2])
    spy = int(argv[3])
    hp = int(argv[4])
    shld = int(argv[5])
    
    result = classify(argv[0], [atk, defe, spy, hp, shld])

    label = result["class_name"].lower()
    
    #sys.exit sends the value to the C++ code
    if (label == "attack"):
        sys.exit(1)
    elif (label == "defend"):
        sys.exit(2)
    elif (label == "advance"):
        sys.exit(3)
    elif (label == "retreat"):
        sys.exit(4)
    else:
        sys.exit(4)

if __name__ == "__main__":
   main(sys.argv[1:])