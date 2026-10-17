# Distributed TSP Solver

![CI](https://github.com/michaelgamston/ga-tsp-platform/actions/workflows/ci.yml/badge.svg)

A cloud-native system that solves the **Travelling Salesman Problem (TSP)** using a
**genetic algorithm**, designed to demonstrate **Docker, Kubernetes and Azure** in a
realistic, scalable architecture.

> 🚧 **Actively in development.** See [Progress](#progress) and [Next steps](#next-steps).

---

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
| API & workers | *(evaluating which langauge to use)* | Accept jobs, queue them, run the engine, report progress |
| Infrastructure | Docker, Kubernetes (AKS), Helm, KEDA | Containerised services with queue-driven autoscaling |
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
  - accepts trailing commas, surrounding whitespace and Windows (`\r\n`) line endings
  - error messages that report the offending value and line number
- **Project structure:** `include/` / `src/` / `tests/` layout, with the engine code built as a
  reusable library (`tsp_core`)
- **CMake build** with compiler warnings enabled
- **21 unit tests** (GoogleTest), covering pass, fail and real-data cases, written partly test-first
- **Continuous integration:** GitHub Actions builds and tests every pull request
- **Workflow:** all changes go through feature branches and pull requests

### 🔨 In progress
- **Genetic algorithm engine:** permutation-based individuals and a tour-length fitness function
  with penalties for unconnected routes

---

## Next steps

1. **Finish the GA engine:** selection, ordered crossover (OX) and mutation, each with unit tests
2. **Engine output:** stream progress (best and average tour length per generation) as JSON lines
3. **API and worker services:** job submission, queueing (Redis) and result storage (Postgres)
4. **Docker:** containerise each service and run everything locally with docker-compose
5. **Kubernetes:** deploy locally, then package with Helm
6. **Azure:** provision AKS and a container registry with Terraform, then deploy through CI/CD
7. **Autoscaling demo:** KEDA scales workers with queue length (0 → N → 0)

---

## Running it locally

**Requirements:** CMake ≥ 3.22 and a C++17 compiler (GCC 11+)

```bash
cd tsp_engine
cmake -S . -B build
cmake --build build

# run the tests
ctest --test-dir build --output-on-failure

# run the engine on an example map
./build/tsp_engine < ../maps/map_05.csv
```

Example maps and the input format are described in [maps/README.md](maps/README.md).

---

## Repository layout
```
.github/workflows/   CI pipeline
maps/                Example distance-matrix maps (with known optimal tour lengths)
tsp_engine/
├── include/         Public headers
├── src/             Engine source code
└── tests/           GoogleTest unit tests
```
