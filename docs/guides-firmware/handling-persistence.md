# Handling Persistence

W4RPBLE manages the storage of **Rules** automatically, but developers often need to store their own configuration (e.g., "Default Color", "Last Relay State").

## The `w4rp` Namespace

The library uses the NVS namespace `w4rp` for its internal consistency.
**Do not** modify keys starting with `rules_` or `active_slot`.

## Helper Methods

The library exposes two public methods for safely accessing NVS within the same namespace. This saves you from initializing NVS handles manually.

### Reading Data
```cpp
String color = w4rp.nvsRead("default_color");
if (color == "") color = "#FF0000"; // Default
```

### Writing Data
```cpp
w4rp.nvsWrite("default_color", "#00FF00");
```
*   **Commit**: The `nvsWrite` method automatically performs `nvs_commit()`.
*   **Performance**: Flash writes are slow (ms). Do not call this in a tight loop. Only call it when values Change.

## Best Practices

1.  **Wear Leveling**: Flash memory has limited write cycles (~100k).
    *   **Bad**: Writing `rpm` to NVS every 100ms.
    *   **Good**: Writing `rpm_max_peak` only when the engine stops.
2.  **Length Limit**: NVS strings are limited in length (max 4000 bytes technically, but practically keep small). For large data, use SPIFFS/LittleFS.
