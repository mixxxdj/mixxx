all

# The same headline in different nested sections is okay (and necessary for
# CHANGELOG.md).
rule 'MD024', :allow_different_nesting => true

# We use ordered lists to make stuff easier to read in a text editor.
rule 'MD029', :style => :ordered

# Not wrapping long lines makes diffs easier to read, especially for prose.
# Instead, we should follow the "one sentence per line" pattern.
exclude_rule 'MD013'

# Dollar signs are useful to indicate shell commands/type and help
# distinguishing wrapped lines from new commands.
exclude_rule 'MD014'

# Indented code blocks are easier to read in a text editor, but don't allow
# specifying a language for syntax highlighting. Therefore both indented and
# fenced code block should be allowed depending on the use case.
exclude_rule 'MD046'
