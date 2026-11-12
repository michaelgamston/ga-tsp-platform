import redis
import os
import subprocess
import json


def start_worker():

    queue = redis.Redis(host=os.getenv("REDIS_HOST", "localhost"), port=6379, decode_responses=True, socket_timeout=5)
    engine_pth = os.getenv("ENGINE_PATH", "../tsp_engine/build/tsp_engine")

    while True:

        
        result = queue.brpop("jobs", timeout=0.5)
        if result:
            _, item = result
            job = json.loads(item)

            pro = subprocess.Popen(
                [engine_pth],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )

            pro.stdin.write(job["map_csv"])
            pro.stdin.close()
            for line in pro.stdout:
                print(json.loads(line))

            result = pro.wait()
            print(result)
            if result != 0: 
                error = pro.stderr.read()
                print(error)

if __name__ == "__main__":
    start_worker()