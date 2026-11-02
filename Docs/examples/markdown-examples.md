# Markdown Examples {#markdown-examples}

> [!NOTE]
> Not all of these examples get translated properly from markdown to html by doxygen, but they'll be readable on github and in a markdown parser like VS Code.

This file contains some helpful examples on how to format markdown (`.md`) files for documentation.

If you are ever unsure, open up this file is Visual Studio Code and Preview it (top left in the tab bar) to see how the source looks before and after formatting.

## Code with Markdown

When we want to make the reader understand when something is code, we write it as an inline **code snippet** or a **code block**. 

### Code snippets

A snippet like `int a = 1` is written between backticks like so:

> \`int a = 1\`

### Code blocks

To write a **code block**, we surround the code with triple backticks and optionally (preferably) the name of the language being typed in next to the first set of backticks. This:

> \`\`\`cpp
> // This is a markdown code block. Here's a function.
> int DoSomething()
> {
>   return 1 + 1;
> }
> \`\`\`

Becomes:

```cpp
// This is a markdown code block. Here's a function.
int DoSomething(){
    return 1 + 1;
}
```

### Source Code snippets

We can also reference code blocks as snippets *in-line with the cpp*. By writing in our source:

```cpp
// do something snippet example
// [do-something]
// This is written directly in the C++ file, see HelloEngine.cpp
int DoSomething()
{
    return 1 + 1;
}
// [do-something]
```

We can reference that snippet in the markdown with `\snippet{lineno} Project/Source.cpp snippet-name`.

`\snippet{lineno} Engine/HelloEngine.cpp do-something` becomes:

\snippet{lineno} Engine/HelloEngine.cpp do-something

> [!NOTE]
> Right now snippets are only looked for in *Samples/*, it doesn't look at the rest of our source code. If we want to change it, we can do so in the  Doxyfile to the right of `EXAMPLE_PATH =`.

## Maths

Sometimes we want to represent maths in our documentation, and I think we can all agree formulas are easier to parse as math than plain text. For this we use [MathJax](https://docs.mathjax.org/en/latest/).

### Inline Maths

MathJax encloses its maths using dollar signs. Inline math is enclosed like so \$a + b = c\$.

If we want to talk about variable $a$ and $b$ we can do that inline with single dollar signs. If we want to explain something more extensively we use a block.

### Block Maths

For more thorough documentation of maths we use blocks, which use *double* dollar signs on each side. `\$\$\int_text_{0}^{2}2x dx\$\$` becomes:

$$\int_\text{0}^{2}2x dx$$

MathJax has support for aligned multiline blocks, but Doxygen does not.

To write an aligned math block that Doxygen can parse, write `\f{align}{/*contents*/\f}`

\f{align}{
    a &= -1\\
    b &= 2\\
    c^2 &= a^2 + b^2\\
        &\leftrightarrow c = \sqrt{a^2+b^2}
          \quad \text{for a right-angle triangle.}\\
    c &= \sqrt{5}
\f}

## Referencing files

### Referencing within a document

To reference headers in a file, simply use `[any name works](#header-in-lowercase)`.

To go to the next section, [click here](#markdown-examples-referencing-other-files).

### Referencing other documents {#markdown-examples-referencing-other-files}

If I want to reference other documentation in this folder I reference them with `[text](#header-tag)`. So to reference **Heading 2** in `example-markdown.md`, in that file I write `## Heading 2 {#example-heading-2}`. To reference in in another file I then write `[Example Heading 2](@ref example-heading-2)` and voila: [Example Heading 2](@ref example-heading-2).

The reference written in the brackets has to be unique.

### Unique doc references {#markdown-examples-unique-references}

To create a unique reference id, write `{#unique-tag-name}` next to the heading. To minimize the risk of id clashes, give it a name that relates to the file it's being written in, like `{#markdown-examples-unique-references}` for this section.

Unique refs also means that the reference survives the heading being changed, reducing the amount of refactoring that has to be done in other files.

### Subpages

In order to not get a single flat list with every single markdown document, we need to include their unique reference as a subpage. Subpages are automatically ordered in the hierarchy inside the html. The index subpages this file, and this file subpages the example file, so the hierarchy becomes `Index>Markdown Examples>Example Markdown`.

The only 'negative' thing is that subpages *must* be referenced and linked in a parent doc using `\subpage reference-id "Link Text"`

\subpage example-markdown-file "Example Markdown File"

### Image Referencing

We can either display images **inline** or **link to them**. Inline linking shows it in the document, linking opens the file like how a url works.

#### Image File Reference

Put an image in `img/` and link it with `[link text](path/to/image/name.png)`. The image has to be relative to `doxygen/html/index.html` so add a `../../` to go back up to root before navigating.

[image](../../Docs/img/runtime-architecture.png)

#### Image Inline Reference

To display an image inline, reference the file and put a `!` in front. `![image](path/to/image/image.png)`. Example:

![image](img/runtime-architecture.png)

## Quotes

To **Quote** something, write a `>` on the same line:

## no-parse

To **NOT** parse a character, add a backslash, like `\#` or `\- Text`.

## Admonitions

*Also called **Alerts** or **Callouts**.*

Sometimes we want to add a visual box. This isn't supported on everything, but both GitHub and Obsidian support **admonitions/callouts**.

To write an admonition, start a quote and put a [!TYPE] on the first line after a space.

```git
> [!NOTE]
> A note.

> [!WARNING]
> There needs to be a space between the \> and the text.

> [!TIP]
> A tip.

> [!CAUTION]
> Caution.

> [!IMPORTANT]
> Important.
```

> [!NOTE]
> A blue whale's call is 188 dB loud. For comparison, standing very close to a jet engine is around 150 dB.

> [!WARNING]
> Successfully failed.

> [!TIP]
> Rinsing with mouth wash after flossing and 30 minutes before brushing brushing is better for your teeth.

> [!CAUTION]
> Spooky Scary Skeletons.

> [!IMPORTANT]
> Take a short break every now and then, it's good for you.

## Mermaid Diagrams

### In Markdown

People aren't fond of these, but markdown supports them so they're here for completeness.

In a markdown context, a mermaid diagram will render if you write

> \`\`\`mermaid
> valid mermaid syntax
> \`\`\`

### In Doxygen

Doxygen doesn't support the backtick version, but they added support for them recently. Instead of the triple backticks you write `@mermaid` and `@endmermaid`.

Write like with the backticks:

> ```
> @mermaid
> valid mermaid syntax
> @endmermaid
> ```

And you end up with something like:

@mermaid
---
title: Hierarchy of the Gupta sitting
---
flowchart TD

A[Gupta]
B[Guests]
C[Axel]
D[Tåny]

A-->B-->C-->D
@endmermaid
