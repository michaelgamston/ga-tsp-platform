import uuid
import redis
import os
import json
from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()
queue = redis.Redis(host=os.getenv("REDIS_HOST", "localhost"), port=6379, decode_responses=True)


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

    queue.lpush("jobs", json.dumps(job_json))

    return {"job_id": job_id}

    