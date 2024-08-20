import os
from glob import glob

# Ensure the necessary environment variables are set up for SDL2
env = Environment()

# Include the SDL2 directory for the include paths and library paths
env.Append(LIBS=['SDL2', 'm'])

# Debug build environment
debug_env = env.Clone()
debug_env.Append(CCFLAGS=['-Wall', '-Og', '-g'])

# Release build environment
release_env = env.Clone()
release_env.Append(CCFLAGS=['-Wall', '-O3'])

# Source and target directories
src_dir = 'src'
bin_dir = 'bin'

# Create a list of all source files in the src directory
source_files = glob(os.path.join(src_dir, '*.c'))

# Create the bin directory if it doesn't exist
if not os.path.exists(bin_dir):
    os.makedirs(bin_dir)

# Compile each source file to an object file in the bin directory
debug_object_files = []
release_object_files = []
for src in source_files:
    debug_obj = os.path.join(bin_dir, os.path.splitext(os.path.basename(src))[0] + '_debug.o')
    release_obj = os.path.join(bin_dir, os.path.splitext(os.path.basename(src))[0] + '_release.o')
    debug_object_files.append(debug_env.Object(target=debug_obj, source=src))
    release_object_files.append(release_env.Object(target=release_obj, source=src))

# Link the object files to create the final executables
debug_env.Program(target='pawocalypse_debug', source=debug_object_files)
release_env.Program(target='pawocalypse_release', source=release_object_files)

# Default target is debug
Default(debug_env.Alias('default', 'pawocalypse_debug'))

# Add a release alias
release_env.Alias('release', 'pawocalypse_release')