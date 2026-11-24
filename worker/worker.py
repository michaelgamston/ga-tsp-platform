import redis
import os
import subprocess
import json
from psycopg_pool import ConnectionPool 

DB_HOST = os.getenv("DB_HOST")
DB_NAME = os.getenv("DB_NAME")
DB_USER = os.getenv("DB_USER")
DB_PASSWORD = os.getenv("DB_PASSWORD")
ENGINE_PTH = os.getenv("ENGINE_PATH", "../tsp_engine/build/tsp_engine")

def start_worker():

    queue = redis.Redis(host=os.getenv("REDIS_HOST", "localhost"), port=6379, decode_responses=True, socket_timeout=5)
    pool = ConnectionPool(kwargs={"host" : DB_HOST, "dbname" : DB_NAME, "user":  DB_USER, "password" : DB_PASSWORD, "sslmode" : "require"}, min_size=1, max_size=1, open=True)

    while True:

        
        result = queue.brpop("jobs", timeout=0.5)
        if result:
            _, item = result
            job = json.loads(item)

            job_id = job["job_id"]

            with pool.connection() as conn:
                conn.execute("UPDATE jobs SET status = 'running' WHERE id = %s", (job_id,))

            pro = subprocess.Popen(
                [ENGINE_PTH],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )

            pro.stdin.write(job["map_csv"])
            pro.stdin.close()
            final_gen = None
            for line in pro.stdout:
                print(json.loads(line))
                final_gen = json.loads(line)

            result = pro.wait()
            print(result)
            if result != 0: 
                error = pro.stderr.read()
                with pool.connection() as conn:
                    conn.execute("""
                                UPDATE jobs 
                                SET status = 'failed', 
                                    error = %s,
                                    finished_at = now()
                                WHERE id = %s""", 
                                 (error, job_id,))
                print(error)
            else: 
                with pool.connection() as conn:
                    conn.execute("""
                                UPDATE jobs 
                                SET status = 'done', 
                                    best_fit = %s,
                                    best_genome = %s,
                                    finished_at = now() 
                                WHERE id = %s""", 
                                    (final_gen["Fitness"], final_gen["Genome"], job_id,))

if __name__ == "__main__":
    start_worker()