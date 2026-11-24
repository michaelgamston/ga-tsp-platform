import uuid
import redis
import os
import json
from fastapi import FastAPI
from fastapi import HTTPException
from pydantic import BaseModel
from psycopg_pool import ConnectionPool 
from psycopg.rows import dict_row

DB_HOST = os.getenv("DB_HOST")
DB_NAME = os.getenv("DB_NAME")
DB_USER = os.getenv("DB_USER")
DB_PASSWORD = os.getenv("DB_PASSWORD")

app = FastAPI()
queue = redis.Redis(host=os.getenv("REDIS_HOST", "localhost"), port=6379, decode_responses=True)
pool = ConnectionPool(kwargs={"host" : DB_HOST, "dbname" : DB_NAME, "user":  DB_USER, "password" : DB_PASSWORD, "sslmode" : "require", "row_factory": dict_row}, min_size=1, max_size=5, open=True)

class JobRequest(BaseModel):
    map_csv : str

@app.get("/health")
def health():
    return {"status": "ok"}

@app.post("/jobs")
def create_job(job: JobRequest):
    job_id = str(uuid.uuid4())

    job_json = {
        "job_id": job_id,
        "map_csv" : job.map_csv
           }

    with pool.connection() as conn:
        conn.execute("INSERT INTO jobs (id, status, map) VALUES (%s, %s, %s)",
                    (job_id, "queued", job.map_csv))

    queue.lpush("jobs", json.dumps(job_json))

    return {"job_id": job_id}

@app.get("/jobs/{job_id}")
def return_job(job_id : uuid.UUID):

    with pool.connection() as conn:
        curosr = conn.execute("SELECT id, status, best_fit, best_genome, error, created_at, finished_at FROM jobs WHERE id = %s", (job_id,))
        row = curosr.fetchone()
    
    if row is None: 
        raise HTTPException(status_code=404, detail="Job not found")

    return row