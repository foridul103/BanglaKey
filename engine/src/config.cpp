#include <banglakey/config.hpp>

namespace banglakey {

ValidationResult EngineConfig::validate() const {
    if (max_buffer_size < 8) {
        return ValidationResult::failure("max_buffer_size cannot be smaller than 8 bytes.");
    }
    if (max_buffer_size > 4096) {
        return ValidationResult::failure("max_buffer_size cannot exceed 4096 bytes.");
    }
    if (max_candidates < 1) {
        return ValidationResult::failure("max_candidates must be at least 1.");
    }
    if (max_candidates > 30) {
        return ValidationResult::failure("max_candidates cannot exceed 30.");
    }
    return ValidationResult::success();
}

} // namespace banglakey
