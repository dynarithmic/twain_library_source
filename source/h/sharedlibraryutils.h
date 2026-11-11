/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2026 Dynarithmic Software.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
    DYNARITHMIC SOFTWARE. DYNARITHMIC SOFTWARE DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
    OF THIRD PARTY RIGHTS.
 */
#ifndef SHAREDLIBRARYUTILS_H
#define SHAREDLIBRARYUTILS_H

#include <boost/dll/shared_library.hpp>
#include <boost/dll/shared_library_load_mode.hpp>
#include <string>
#include <vector>
#include <utility>
#include <filesystem>

namespace dynarithmic
{
    template <typename FuncPtr>
    std::pair<bool, FuncPtr> GetSharedLibraryFunction(const boost::dll::shared_library& library, const char* name, 
                                                      FuncPtr& function)
    {
        static_assert(std::is_pointer_v<FuncPtr>);
        static_assert(std::is_function_v<std::remove_pointer_t<FuncPtr>>);
        using FuncType = std::remove_pointer_t<FuncPtr>;
        function = std::addressof(library.get<FuncType>(name));
        return { function != nullptr, function };
    }
    namespace fs = std::filesystem;

    template <typename ErrorCodeType = std::error_code>
    int LoadSharedLibrary(boost::dll::shared_library& libloader, const std::wstring& fNameTotal)
    {
        boost::dll::load_mode::type loadingtype = boost::dll::load_mode::search_system_folders;
        fs::path dllPath(fNameTotal);
        if (dllPath.has_parent_path())
        {
            dllPath = fs::absolute(fs::path(fNameTotal));
            loadingtype =  static_cast<boost::dll::load_mode::type>(LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        }
        dllPath.make_preferred();
        ErrorCodeType ec;
        libloader.load(dllPath.wstring(), ec, loadingtype);
        return ec.value();
    }

    class DynamicLibraryLoader
    {
        public:
            struct FunctionEntry
            {
                std::string dllName;
                std::string functionName;
                std::function<bool(DynamicLibraryLoader&)> resolve;
            };

            using FunctionTable = std::vector<FunctionEntry>;

            template<typename Functions, typename FunctionPointer>
            static FunctionEntry RegisterFunction(const std::string& dllName,const std::string& functionName,
                Functions& functions,FunctionPointer Functions::* member)
            {
                static_assert(std::is_pointer_v<FunctionPointer>, "Function member must be a function pointer");

                return 
                {
                    dllName,functionName,[&functions, member, dllName, functionName]
                            (DynamicLibraryLoader& loader) -> bool
                        {
                            return loader.GetFunction(dllName,functionName,functions.*member);
                        }
                };
            }

            bool ResolveFunctions(const FunctionTable& table)
            {
                for (const auto& entry : table)
                {
                    if (!entry.resolve(*this))
                        return false;
                }

                return true;
            }

            DynamicLibraryLoader() = default;

            ~DynamicLibraryLoader()
            {
                Unload();
            }

            DynamicLibraryLoader(const DynamicLibraryLoader&) = delete;
            DynamicLibraryLoader& operator=(const DynamicLibraryLoader&) = delete;


            DynamicLibraryLoader(DynamicLibraryLoader&& other) noexcept
            {
                *this = std::move(other);
            }

            DynamicLibraryLoader& operator=(DynamicLibraryLoader&& other) noexcept
            {
                if (this != &other)
                {
                    Unload();

                    m_libraries = std::move(other.m_libraries);
                    m_loaded = other.m_loaded;
                    m_lastError = other.m_lastError;
                    m_failedName = std::move(other.m_failedName);

                    // Reset the moved-from object.
                    other.m_loaded = false;
                    other.m_lastError = ERROR_SUCCESS;
                    other.m_failedName.clear();
                }

                return *this;
            }

            bool Load(const std::vector<std::string>& dllNames, const std::wstring& directory = L"");

            template<typename FunctionPointer>
            bool GetFunction(const std::string& dllName,const std::string& functionName, FunctionPointer& output)
            {
                output = nullptr;

                for (auto& entry : m_libraries)
                {
                    if (entry.first == dllName)
                    {
                        if (!entry.second.has(functionName))
                        {
                            m_failedName = functionName;
                            m_lastError = ERROR_PROC_NOT_FOUND;
                            return false;
                        }

                        try
                        {
                            output = &entry.second.get<std::remove_pointer_t<FunctionPointer>>(functionName);
                            return true;
                        }
                        catch (const std::exception&)
                        {
                            m_failedName = functionName;
                            m_lastError = ERROR_PROC_NOT_FOUND;
                            return false;
                        }
                    }
                }

                m_failedName = dllName;
                m_lastError = ERROR_MOD_NOT_FOUND;
                return false;
            }

            void Unload() noexcept
            {
                // Release in reverse loading order.
                while (!m_libraries.empty())
                    m_libraries.pop_back();

                m_loaded = false;
            }

            bool IsLoaded() const noexcept
            {
                return m_loaded;
            }

            DWORD GetLastErrorCode() const noexcept
            {
                return m_lastError;
            }

            const std::string& GetFailedName() const noexcept
            {
                return m_failedName;
            }

        private:
            using LibraryEntry = std::pair<std::string, boost::dll::shared_library>;

            std::vector<LibraryEntry> m_libraries;

            bool m_loaded = false;
            DWORD m_lastError = ERROR_SUCCESS;
            std::string m_failedName;
    };
}
#endif
