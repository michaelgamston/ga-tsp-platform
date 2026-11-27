// All API calls go to /api/..., which nginx forwards to the FastAPI service.
const API = "/api";

// How often to check a running job, in milliseconds
const POLL_INTERVAL_MS = 1000;
// Give up checking after this many failed requests in a row
const MAX_POLL_FAILURES = 5;

// ---------- Find the page elements we need (by their id in index.html) ----------
const healthBtn = document.getElementById("health-btn");
const healthStatus = document.getElementById("health-status");

const exampleSelect = document.getElementById("example-map");
const fileInput = document.getElementById("map-file");
const mapBox = document.getElementById("map-csv");

const jobForm = document.getElementById("job-form");
const submitBtn = document.getElementById("submit-btn");
const formError = document.getElementById("form-error");

const jobPanel = document.getElementById("job-panel");
const jobIdEl = document.getElementById("job-id");
const jobStatusEl = document.getElementById("job-status");
const jobFitnessEl = document.getElementById("job-fitness");
const jobRouteEl = document.getElementById("job-route");
const jobTimeEl = document.getElementById("job-time");
const jobErrorEl = document.getElementById("job-error");

// The optimal route length of the chosen example map, if known (from data-optimal)
let optimalLength = null;

// ---------- API health check ----------
async function checkHealth() {
  healthStatus.textContent = "checking…";
  try {
    const response = await fetch(`${API}/health`);
    const body = await response.json();
    healthStatus.textContent = response.ok ? body.status : `error ${response.status}`;
  } catch {
    // fetch throws if the request couldn't be made at all (e.g. nginx or the API is down)
    healthStatus.textContent = "unreachable";
  }
}

healthBtn.addEventListener("click", checkHealth);
checkHealth(); // also check once when the page loads

// ---------- Choosing an example map ----------
exampleSelect.addEventListener("change", async () => {
  hideFormError();
  fileInput.value = ""; // clear any uploaded file, so only one source is shown as chosen

  const option = exampleSelect.selectedOptions[0];
  optimalLength = option.dataset.optimal ? Number(option.dataset.optimal) : null;

  if (exampleSelect.value === "") return; // "Choose an example…" was picked

  try {
    const response = await fetch(exampleSelect.value);
    if (!response.ok) throw new Error(`could not load ${exampleSelect.value} (${response.status})`);
    mapBox.value = (await response.text()).trim();
  } catch (err) {
    showFormError(`Couldn't load the example map: ${err.message}`);
  }
});

// ---------- Uploading a map file ----------
fileInput.addEventListener("change", async () => {
  hideFormError();
  exampleSelect.value = ""; // reset the dropdown
  optimalLength = null;     // an uploaded map has no known optimal length

  const file = fileInput.files[0];
  if (!file) return; // the user cancelled the file picker

  mapBox.value = (await file.text()).trim();
});

// Editing the map by hand means it may no longer match the chosen example
mapBox.addEventListener("input", () => {
  if (exampleSelect.value !== "") {
    exampleSelect.value = "";
    optimalLength = null;
  }
});

// ---------- Submitting a job ----------
jobForm.addEventListener("submit", async (event) => {
  event.preventDefault(); // stop the browser's default behaviour (reloading the page)
  hideFormError();

  const mapCsv = mapBox.value.trim();
  if (mapCsv === "") {
    showFormError("Please choose, upload or paste a map first.");
    return;
  }

  submitBtn.disabled = true;

  try {
    const response = await fetch(`${API}/jobs`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ map_csv: mapCsv }),
    });

    if (!response.ok) {
      throw new Error(await errorMessageFrom(response));
    }

    const { job_id } = await response.json();
    startJobDisplay(job_id);
    pollJob(job_id, 0);
  } catch (err) {
    showFormError(`Couldn't submit the job: ${err.message}`);
    submitBtn.disabled = false;
  }
});

// ---------- Checking a job until it finishes ----------
async function pollJob(jobId, failures) {
  try {
    const response = await fetch(`${API}/jobs/${jobId}`);
    if (!response.ok) {
      throw new Error(await errorMessageFrom(response));
    }

    const job = await response.json();
    showJob(job);

    if (job.status === "done" || job.status === "failed") {
      submitBtn.disabled = false; // finished, so allow another job
      return;                     // stop checking
    }

    failures = 0; // a successful check resets the failure count
  } catch (err) {
    failures += 1;
    if (failures >= MAX_POLL_FAILURES) {
      showJobError(`Stopped checking the job: ${err.message}`);
      submitBtn.disabled = false;
      return;
    }
  }

  // Not finished yet: check again after a short wait
  setTimeout(() => pollJob(jobId, failures), POLL_INTERVAL_MS);
}

// ---------- Updating the result panel ----------
function startJobDisplay(jobId) {
  jobPanel.hidden = false;
  jobIdEl.textContent = jobId;
  setStatus("queued");
  jobFitnessEl.textContent = "–";
  jobRouteEl.textContent = "–";
  jobTimeEl.textContent = "–";
  jobErrorEl.hidden = true;
  jobErrorEl.textContent = "";
}

function showJob(job) {
  setStatus(job.status);

  if (job.status === "done") {
    jobFitnessEl.textContent = formatFitness(job.best_fit);
    jobRouteEl.textContent = formatRoute(job.best_genome);
    jobTimeEl.textContent = formatDuration(job.created_at, job.finished_at);
  }

  if (job.status === "failed") {
    jobTimeEl.textContent = formatDuration(job.created_at, job.finished_at);
    showJobError(job.error || "The job failed without an error message.");
  }
}

function setStatus(status) {
  jobStatusEl.textContent = status;
  jobStatusEl.dataset.status = status; // style.css colours the badge from this
}

function showJobError(message) {
  jobErrorEl.textContent = message;
  jobErrorEl.hidden = false;
}

// ---------- Form error line ----------
function showFormError(message) {
  formError.textContent = message;
  formError.hidden = false;
}

function hideFormError() {
  formError.textContent = "";
  formError.hidden = true;
}

// ---------- Formatting helpers ----------

// e.g. 282, or "282 (optimal: 282)" for an example map
function formatFitness(fitness) {
  if (fitness === null || fitness === undefined) return "–";
  return optimalLength === null ? `${fitness}` : `${fitness} (optimal: ${optimalLength})`;
}

// e.g. [1, 6, 4] -> "1 → 6 → 4 → 1" (the route returns to where it started)
function formatRoute(genome) {
  if (!Array.isArray(genome) || genome.length === 0) return "–";
  return [...genome, genome[0]].join(" → ");
}

// Time between the job being created and finishing, e.g. "1.4 s"
function formatDuration(createdAt, finishedAt) {
  if (!createdAt || !finishedAt) return "–";
  const seconds = (new Date(finishedAt) - new Date(createdAt)) / 1000;
  return `${seconds.toFixed(1)} s`;
}

// Turn an API error response into readable text.
// FastAPI sends {"detail": "..."} for errors like 404, and {"detail": [...]} for 422 validation errors.
async function errorMessageFrom(response) {
  try {
    const body = await response.json();
    if (typeof body.detail === "string") return body.detail;
    if (Array.isArray(body.detail)) return body.detail.map((d) => d.msg).join("; ");
  } catch {
    // the response wasn't JSON (e.g. an nginx error page)
  }
  return `HTTP ${response.status}`;
}
