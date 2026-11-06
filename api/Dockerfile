FROM python:3.11-slim

WORKDIR /api

COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt
#split the copy in two 
#api.py will change, we only want docker to rebuild from this step downwards and not
#install pip requirements again each time api.py changes 
RUN useradd --no-create-home api_usr

COPY api.py /api

USER api_usr

ENTRYPOINT ["uvicorn", "api:app", "--host", "0.0.0.0", "--port", "8000"]
