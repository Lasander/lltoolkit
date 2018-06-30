# Add all external dependencies to the project.
#
# To include their libraries and include paths to a local target compilation
# and linking, add the desired target with target_link_libraries(), e.g.:
#   in myLib/CMakeLists.txt:
#     target_link_libraries(myLib external-lib)
add_subdirectory(googletest)

###############################################################################
# To add new external dependencies from other projects available in git:
#
# 1. Create a subdirectory for the external project. This will allow making a
#    cmake wrapper.
#
# 2. Add a new git submodule under the subdirectory directory:
#      git submodule add --name <subdirectory> https://url.to.new.dep
#    Note: To create shallow clone (of the main branch) use "--depth 1" and
#          additionally add "shallow = true" to the generated .gitsubmodules
#          entry.
#
# 3. Checkout the desired version of the submodule. E.g if the dependency has
#    release with tag "v4.5", you can check it out by running in the submodule
#    directory:
#      git checkout v4.5
#
# 4. Add a CMakeLists.txt to the subdirectory, which should
#    [if cmake project]
#      add the submodule directory with add_subdirectory()
#    [if not cmake project]
#      define the libraries and related includes paths for the libraries
#
# 5. Add a line in the above list to include the dependency:
#      add_subdirectory(new-external-lib)
#
# 6. Make a git commit, which adds the new submodule directory and a change to
#    .gitmodules in the root of the project (which should have changed)
#
# 7. Use the new library in the desired local targets using
#    target_link_libraries() (see above.)
#
# Note: For non-git projects, let's look at the best way to include the source
#       code in the repo. Could be e.g. either by copying and including it in
#       the repo itself, or by creating some download/extract targets.
#
# Note: If the repository has a very large size, consider doing a shallow
#       submodule by adding "shallow = true" to the .gitmodules configuraion.
#       This will however only work if the desired branch is the default branch
#       of the remote. If it is not, we can make a fork of the original project
#       to make it so.
###############################################################################
