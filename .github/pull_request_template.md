## Summary

<!-- What does this PR change and why? Keep it to a few sentences. -->

## Type of change

- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that changes existing behavior)
- [ ] Refactor / cleanup (no functional change)
- [ ] Documentation / chore

## Hardware impact

<!-- Does this affect wiring, pin assignments, or supported hardware? -->

- [ ] No hardware impact
- [ ] Pin assignments changed (update README and config.h)
- [ ] New module / sensor / library added
- [ ] Memory footprint changed significantly (note RAM/Flash deltas below)

## Test plan

<!-- How was this verified? Check all that apply. -->

- [ ] Builds clean with `pio run`
- [ ] Tested in Wokwi simulator (`wokwi.toml` / `diagram.json`)
- [ ] Tested on real Arduino Nano + GPS hardware
- [ ] Time display verified (12H and 24H formats)
- [ ] Date display verified
- [ ] GPS location display verified
- [ ] Rain effect / GPS-loss fallback verified
- [ ] Brightness day/night transition verified

## Memory footprint (if changed)

<!-- Paste `pio run` output showing RAM/Flash usage before and after. -->

```
Before: RAM:  ___ /2048 bytes   Flash: _____ /32256 bytes
After:  RAM:  ___ /2048 bytes   Flash: _____ /32256 bytes
```

## Related issues

<!-- Link any related issues, e.g. "Closes #12" -->

## Screenshots / video (optional)

<!-- Drag and drop images or video showing the change on the matrix. -->
