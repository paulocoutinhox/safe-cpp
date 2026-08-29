# Contribution

Keep changes focused, modern and compatible with the current architecture. C++20 is the minimum. Run `python3 make.py format` before tests and `python3 make.py test` before submitting changes.

New modules require a focused namespace, public headers under `include/safe_cpp`, mirrored runtime code under `src` when runtime implementation exists, tests through GoogleTest and a concise module document. Avoid compatibility layers for replaced APIs and remove obsolete code instead of preserving parallel behavior.
