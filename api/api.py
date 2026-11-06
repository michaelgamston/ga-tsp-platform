import uuid

from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

class JobRequest(BaseModel):
    map_csv : str

@app.get("/health")
def health():
    return {"status": "ok"}

@app.post("/jobs")
def create_job(job: JobRequest):
    job_id = str(uuid.uuid4())
    return {"job_id" : job_id}

    