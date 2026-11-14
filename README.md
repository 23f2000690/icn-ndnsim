# ICN project: NS-3 + ndnSIM

Wired Information-Centric Networking lab. NS-3 with ndnSIM runs in Docker, because ndnSIM builds on Linux. The topology scenario stays in this folder, so you edit it on Windows and rerun.

## What is simulated

Every node runs the NDN forwarder, so each one has a Content Store (CS), a Pending Interest Table (PIT), and a Forwarding Information Base (FIB).

```
        cr_ul -------- cr_ur ---- producer1     /C1 /C3 /C4
          / \            / \
         /   \   cr_ctr /   \
        /     \    |    /     \
   client1,2   cr_lc      cr_rc
    on cr_lc      |          /
        \       / \        /
         \     /   \      /
        cr_ll -------- cr_lr ---- server2       /C2 /C5 /C6
```

- `client1` and `client2` attach to `cr_lc` (left-centre perimeter router).
- Seven content routers: hex perimeter (`cr_ul`, `cr_ur`, `cr_rc`, `cr_lr`, `cr_ll`, `cr_lc`) plus centre `cr_ctr` (linked to the four corner routers only).
- `producer1` on `cr_ur` serves `/C1`, `/C3`, `/C4`. `server2` on `cr_lr` serves `/C2`, `/C5`, `/C6`.
- Interests are forwarded with NFD best-route. FIB entries are the name prefixes above.
- The CS uses LRU and keeps a copy of returning Data at every hop (leave copy everywhere).

`client2` starts asking for `/C1` a few seconds after `client1`, so some of those Interests can be satisfied from a router cache.

The chalk diagram is the source for these roles. If a link on the board differs, change the matching `Install(...)` line in `scenarios/wired-topology.cc`.

## One-time setup

1. Start Docker Desktop and wait until it says it is running.
2. From this folder:

```powershell
.\scripts\run.ps1
```

The first run builds an image: it clones the ndnSIM ns-3 fork and ndnSIM (including ndn-cxx and NFD) and compiles them. That often takes 30–60 minutes and several gigabytes of disk. Later runs only recompile the scenario.

## Run again after an edit

```powershell
.\scripts\run.ps1
.\scripts\run.ps1 --frequency=10 --csSize=50 --simTime=20
```

Useful arguments, all optional:

| Argument | Default | Meaning |
| --- | --- | --- |
| `--frequency` | 2 | Interests per second for each consumer prefix |
| `--csSize` | 100 | CS size in packets, on every node |
| `--csPolicy` | `nfd::cs::lru` | `nfd::cs::lru` or `nfd::cs::priority_fifo` |
| `--strategy` | best-route | NFD strategy name, installed for `/` |
| `--payload` | 1024 | Data payload size in bytes |
| `--simTime` | 15 | Seconds |

A shell inside the simulator, with this folder's scenarios mounted at `scratch/`:

```powershell
.\scripts\shell.ps1
```

The tree inside the container is `/opt/ndnSIM/ns-3`. Upstream examples are already built; `./waf --run=ndn-simple` runs one.

## Results

After a run, `results/` contains:

- `l3-rate-trace.txt` — Interest and Data rates per node and face
- `cs-trace.txt` — Content Store hits and misses
- `app-delay-trace.txt` — delay of each satisfied Interest

The console lists node ids. Those ids are the `Node` column in the trace files.

## What this covers, and what is next

This is the installation and the small wired topology. The later algorithm (popularity or probability caching, leave copy down, a game-based forwarding strategy, or OSPFN / LSR) is a change inside `scenarios/wired-topology.cc`, not a new simulator install. Best-route and LRU are the baseline those algorithms would replace.
