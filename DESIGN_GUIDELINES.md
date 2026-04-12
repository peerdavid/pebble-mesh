# Design Guidelines

Guidelines that shape how Pebble Mesh is designed and developed.

## Date & Time is always first

The time (and date) is centered, large, and always visible. Whether a notification is showing, the weather forecast is open, or the user just glances at their wrist — the time must always be readable at a glance.

## Readability Through Contrast

Every text element is surrounded by a thin contrast outline to ensure legibility against any background. For example, in the light theme, where the background is gray, black text gets a white outline to stay sharp and readable. But also this helps to make elements more readable on top of the mesh pattern.

## Icons
All icons are pdc icons (convert from SVG with https://pdc-tool.heikobehrens.com/). This way we can scale the icon easily to e.g. emery without having something pixelized. Also whenever an icon is interactive (e.g. the battery icon), the interactive part is gray.

## 100% Community-Driven and Open Source

When users request features, we try to make them happen (its not always possible but often I would say) — even if they go against the default look. For example, a user with aging eyes asked to disable the mesh to improve readability. To ensure everyone can enjoy their preferred version, features like these are always optional and configurable in the settings.

## Have Fun

This is a hobby project. It started as something built just for personal use, shared without any expectations. But learning that others enjoy the watchface too has been pure motivation. Even features that aren't personally needed are worth building — knowing they help others enjoy their Pebble makes it all worthwhile.