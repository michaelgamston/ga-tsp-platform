# Distributed TSP Solver

![CI](https://github.com/michaelgamston/ga-tsp-platform/actions/workflows/ci.yml/badge.svg)

A cloud-native system that solves the **Travelling Salesman Problem (TSP)** using a
**genetic algorithm**, designed to demonstrate **Docker, Kubernetes and Azure** in a
realistic, scalable architecture.

> 🚧 **Actively in development.** See [Progress](#progress) and [Next steps](#next-steps).

---

**Disclaimer** 
The project has used AI to format this README to improve flow and readability. However, all code and development decisions have been made by me. 

## The project

Users submit a map (a matrix of distances between locations) and the system searches
for the shortest route that visits every location once. Each request becomes a job on a
queue, processed by worker containers that **scale automatically with demand**: more
jobs start more workers, and the workers scale back to zero when the queue is empty.

### Planned architecture
```
Web page → API → Job queue (Redis) → Worker containers → C++ GA engine
                       ↘  Postgres (maps, jobs, results)  ↙
```

| Component | Technology | Role |
|---|---|---|
| Solver engine | C++17, CMake, GoogleTest | Evolves candidate routes using a genetic algorithm |
| API & workers | Python w/ FastAPI, Redis | Accept jobs, queue them, run the engine, report progress |
| Infrastructure | Docker, Kubernetes (AKS) | Containerised services with queue-driven autoscaling |
| Cloud & delivery | Azure, Terraform, GitHub Actions | Infrastructure as code; automated build, test and deploy |

---

## What I'm aiming to learn

- **Containerisation:** packaging multiple services with Docker; multi-stage builds
- **Orchestration:** deploying and autoscaling workloads on Kubernetes
- **Cloud infrastructure:** provisioning Azure resources reproducibly with Terraform
- **CI/CD:** automated pipelines that test and deploy every change
- **Modern C++:** project structure, build systems, input validation, unit testing
- **Professional workflow:** feature branches, pull requests, protected `main`, test-driven development

---

## Progress

### ✅ Done
- **Distance-matrix parser** with thorough input validation:
  - rejects non-numeric values, decimals, values that are too large, empty cells, empty input,
    non-square matrices and invalid negative values
  - enforces a map size of 3–50 nodes, which keeps tour lengths well within integer limits
  - accepts trailing commas, surrounding whitespace and Windows (`\r\n`) line endings
  - error messages that report the offending value and line number
- **`Individual` (a candidate route)**, encapsulated in its own class:
  - genome is a random permutation of the map's nodes, reproducible from a seed
  - fitness is the total tour length (including the return to the start), with a penalty for
    every unconnected edge, and is recalculated automatically whenever the genome changes
  - private state behind getters and a validating setter, which rejects duplicate nodes,
    out-of-range nodes and genomes of the wrong length, and leaves the route unchanged when it does
- **Genetic algorithm engine**, with each operator a self-contained, separately tested function:
  - **tournament selection** (tournament size 3) builds a mating pool that favours shorter routes
  - **ordered crossover (OX)** combines two parent routes and always produces a valid route
  - **inversion mutation** reverses a random section of a route, with a configurable mutation rate
  - **elitism** carries the best route into the next generation, so the best result never gets worse
  - **reproducible runs:** the same seed and settings always give the same result
  - **finds the proven optimal routes** on the 5- and 8-city example maps, and avoids
    unconnected edges on the 20-city map
- **Progress output:** one JSON line per generation (generation, best fitness, best route),
  ready for the worker service to stream to the website
- **Project structure:** `include/` / `src/` / `tests/` layout, with the engine code built as a
  reusable library (`tsp_core`)
- **CMake build** with compiler warnings enabled
- **109 unit tests** (GoogleTest) across the parser, `Individual` and the genetic algorithm, covering
  pass, fail, boundary, property-based (e.g. "crossover always returns a valid route" over 1,000
  random cases) and real-data cases, written largely test-first. These tests caught real bugs
  before the code was merged, including crashes from out-of-range memory access and a
  dangling reference
- **Continuous integration:** GitHub Actions builds and tests every pull request, then builds
  the Docker image and smoke-tests it on an example map
- **Workflow:** all changes go through feature branches and reviewed pull requests
- **Command-line entry point:** connecting `main` to the engine, so the population size,
  generations, mutation rate and seed can be passed as arguments and the map read from stdin
- **Docker:** containerise the engine with a multi-stage build

### 🔨 In progress

- **API and worker services:** job submission, queueing (Redis) and result storage (Azure Postgres)
- **docker-compose:** run every service together locally

---

## Next steps
- **Light weight frontend** A user interface to interact with the engine
- **Kubernetes:** deploy locally, then package with Helm
- **Azure:** provision AKS and a container registry with Terraform, then deploy through CI/CD 
- **Autoscaling demo:** KEDA scales workers with queue length (0 → N → 0)

---

## Running it locally

### With Docker

**Requirements:** Docker

```bash
cd tsp_engine
docker build -t tsp-engine .
docker run -i --rm tsp-engine --pop 100 --iter 300 < ../maps/map_08.csv
```

The `-i` flag is required, because the map is passed in through stdin.

### Without Docker

**Requirements:** CMake ≥ 3.22 and a C++17 compiler (GCC 11+)

```bash
cd tsp_engine
cmake -S . -B build
cmake --build build

# run the tests
ctest --test-dir build --output-on-failure

# run the engine
./build/tsp_engine --pop 100 --iter 300 < ../maps/map_08.csv
```

### Options

| Flag | Meaning | Default |
|---|---|---|
| `--pop` | Population size (minimum 3) | 100 |
| `--iter` | Number of generations (minimum 1) | 50 |
| `--mut_rate` | Mutation rate (0 to 1) | 0.1 |
| `--seed` | Random seed, for repeatable runs | 42 |

### Output

The engine prints one JSON line per generation, showing the best route found so far:

```
{ "Gen" : 300 , "Fitness" : 282 , "Genome" : [1,6,4,7,0,2,3,5] }
```

Example maps and the input format are described in [maps/README.md](maps/README.md).

---

## Repository layout
```
.github/workflows/   CI pipeline
api/                 Backend API 
maps/                Example distance-matrix maps (with known optimal tour lengths)
tsp_engine/
├── include/         Public headers
├── src/             Engine source code
├── tests/           GoogleTest unit tests
└── Dockerfile       Multi-stage build for the engine image
docker-compose.yml   Container orchestration file
```
