#include <contracts>

// MinGW's weak default-handler symbol can fail to link. A strong definition
// forwards to the same library handler and preserves its diagnostics.
auto handle_contract_violation(std::contracts::contract_violation const& violation) -> void {
    std::contracts::invoke_default_contract_violation_handler(violation);
}
