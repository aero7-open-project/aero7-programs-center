# Architecture

The Qt Widgets presentation layer depends on an abstract `PackageBackend`, never on scattered process calls. `AlpmBackend` owns the read-only libalpm handle; `Metadata` enriches package records; `PackageTransaction` is the single transaction coordinator; and the privileged helper is the only component allowed to mutate packages. The design leaves room for later provider implementations without mixing their trust models.
