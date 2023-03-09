module;
#include <boost/exception/all.hpp>
#include <string>
export module FilesystemException;

namespace Filesystem
{
	export typedef boost::error_info<struct FilePathErrorInfo, std::wstring> FilePathInfo;
	export typedef boost::error_info<struct FileNameErrorInfo, std::wstring> FileNameInfo;

	export struct FilesystemException : virtual boost::exception, virtual std::exception {
	public:
		FilesystemException(const char* what);
		FilesystemException(std::string what);
		virtual const char* what() const noexcept override;
	private:
		std::string m_what;
	};
}