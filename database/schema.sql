CREATE TABLE IF NOT EXISTS jobs (
    id UUID PRIMARY KEY NOT NULL,
    status TEXT CHECK(status IN ('queued', 'running', 'done', 'failed')) NOT NULL,
    map TEXT NOT NULL,
    best_fit INTEGER,
    best_genome INTEGER[],
    error TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
    finished_at TIMESTAMPTZ
);