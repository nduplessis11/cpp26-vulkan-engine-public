# Debugging playbook

1. Reproduce the smallest deterministic case.
2. Record the command, backend, GPU and driver, validation state, and last good commit.
3. Read the first meaningful error rather than its cascade.
4. State one hypothesis and the observation that would falsify it.
5. Inspect ownership, parent lifetime, resource state, queue, and completion token.
6. Add a temporary invariant, counter, or debug name rather than sleeps or global waits.
7. Compare CPU intent with a GPU capture when the command stream is suspect.
8. Add regression proof.
9. Record the learned invariant in `state/LEARNING_LOG.md`.

`vkDeviceWaitIdle` may be a temporary labeled diagnostic, not the final lifetime strategy.
