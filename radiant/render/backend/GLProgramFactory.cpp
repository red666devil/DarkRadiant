#include "GLProgramFactory.h"
#include "glprogram/ARBBumpProgram.h"
#include "glprogram/ARBDepthFillProgram.h"

#include "iregistry.h"
#include "os/file.h"
#include "string/string.h"
#include "debugging/debugging.h"

#include <fstream>

namespace render
{

// Constructor, populates map with GLProgram instances
GLProgramFactory::GLProgramFactory()
{
	_map.insert(std::make_pair("depthFill", new ARBDepthFillProgram())); 
	_map.insert(std::make_pair("bumpMap", new ARBBumpProgram()));
}

// Return static GLProgramFactory instance
GLProgramFactory& GLProgramFactory::getInstance() {
	static GLProgramFactory _instance;
	return _instance;
}

// Lookup a named program in the singleton instance
GLProgramPtr GLProgramFactory::getProgram(const std::string& name) {
	
	// Reference to static instance's map
	ProgramMap& map = getInstance()._map;
	
	// Lookup the program, if not found throw an exception
	ProgramMap::iterator i = map.find(name);
	if (i != map.end())
		return i->second;
	else
		throw std::runtime_error("GLProgramFactory: failed to find program "
								 + name);
}

// Realise the program factory.
void GLProgramFactory::realise() {
	
	// Get static map
	ProgramMap& map = getInstance()._map;
	
	// Realise each GLProgram in the map
	for (ProgramMap::iterator i = map.begin();
		 i != map.end();
		 ++i)
	{
		i->second->create();
	}
}

// Unrealise the program factory.
void GLProgramFactory::unrealise() {
	
	// Get static map
	ProgramMap& map = getInstance()._map;
	
	// Destroy each GLProgram in the map
	for (ProgramMap::iterator i = map.begin();
		 i != map.end();
		 ++i)
	{
		i->second->destroy();
	}
}

// Get file as a char buffer
GLProgramFactory::CharBufPtr 
GLProgramFactory::getFileAsBuffer(const std::string& filename)
{
    // Get absolute path from filename
    std::string absFileName = getGLProgramPath(filename);

    // Open the file
	std::size_t size = file_size(absFileName.c_str());
	std::ifstream file(absFileName.c_str());
	
    // Throw an exception if the file could not be found
	if (!file.is_open())
    {
        throw std::runtime_error(
            "GLProgramFactory::createARBProgram() failed to open file: "
            + absFileName
        );
    }
	
    // Read the file data into a buffer
	CharBufPtr buffer(new std::vector<char>(size));
    assert(buffer->size() == size);
	file.read(&buffer->front(), size);

    // Close file and return buffer
    file.close();
    return buffer;
}

#ifdef RADIANT_USE_GLSL

GLuint GLProgramFactory::createGLSLProgram(const std::string& vFile,
                                           const std::string& fFile)
{
    return 0;
}

#else

GLuint GLProgramFactory::createARBProgram(const std::string& filename,
                                          GLenum type) 
{
    // Get the file contents
    CharBufPtr buffer = getFileAsBuffer(filename);

    // Bind the program data into OpenGL
    GlobalOpenGL_debugAssertNoErrors();

    GLuint programID;
    glGenProgramsARB(1, &programID);
    glBindProgramARB(type, programID);

	glProgramStringARB(
        type,
        GL_PROGRAM_FORMAT_ASCII_ARB,
        GLsizei(buffer->size()),
        &buffer->front()
    );

    // Check for GL errors and throw exception if there is a problem
	if (GL_INVALID_OPERATION == glGetError()) 
    {
		GLint errPos;
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errPos);
		const GLubyte* errString = glGetString(GL_PROGRAM_ERROR_STRING_ARB);

        // Construct user-readable error string
        std::string error("GL program error: ");
        error += filename + "(" + intToStr(errPos) + "): \n\n";
        error += std::string(reinterpret_cast<const char*>(errString));

        // Throw exception
        throw std::logic_error(error);
	}

    // Return the new program
    return programID;
}

#endif // RADIANT_USE_GLSL

// Get the path of a GL program file
std::string GLProgramFactory::getGLProgramPath(const std::string& progName)
{
    // Determine the root path of the GL programs
#if defined(POSIX) && defined (PKGDATADIR)
    std::string glProgRoot = std::string(PKGDATADIR) + "/";
#else
    std::string glProgRoot = GlobalRegistry().get("user/paths/appPath");
#endif

    // Append the requested filename with the "gl/" directory.
    return glProgRoot + "gl/" + progName;
}

} // namespace render
