/// @author Máté Kelemen

// --- STL Includes ---
#include <memory> // std::unique_ptr
#include <string> // std::string


namespace Kratos::Executables {


class ApplicationLoader
{
public:
    ApplicationLoader();

    ~ApplicationLoader();

    void Load(const std::string& rApplicationName);

    void Unload(const std::string& rApplicationName);

private:
    struct Impl;
    std::unique_ptr<Impl> mpImpl;
}; // class ApplicationLoader


} // namespace Kratos::Executables
