# Doc/build-doc.ps1
# Concatene README.md + Doc/WORKFLOW.md + Doc/Systems/README.md + Doc/Systems/*.md en un seul
# fichier HTML via Pandoc. Les fichiers systemes sont "descendus" d'un niveau de titre (H1->H2,
# H2->H3, etc.) avant la fusion, pour qu'ils s'imbriquent vraiment sous "Reference technique"
# (H1) au lieu de rester des H1 independants relies par des liens relatifs -- un lien du style
# [Personnage](Character.md) casserait de toute facon une fois fusionne (le fichier de sortie ne
# vit plus dans Doc/Systems/). Les fichiers sources sur disque ne sont jamais modifies -- le
# decalage n'existe que dans un fichier temporaire assemble pour cette generation.
# Consequence mineure acceptee : le sommaire auto (--toc) montre aussi les sous-titres H3 de
# README.md/WORKFLOW.md (un peu plus long), pas seulement ceux de la reference technique.
#
# Limite connue : les diagrammes Mermaid sortent en bloc de code brut, pas en image rendue (pas
# assez de diagrammes dans le projet aujourd'hui pour justifier une dependance Node.js/mermaid-cli
# supplementaire -- a revisiter si ce doc est un jour partage a quelqu'un d'exterieur).
#
# Prerequis (une fois) : winget install --id JohnMacFarlane.Pandoc -e
#
# Variante PDF : remplacer l'extension de $Out par .pdf (necessite en plus un moteur PDF, ex.
# MiKTeX/wkhtmltopdf, non couvert par ce script).

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$Out = Join-Path $RepoRoot "Doc\Mage-Doc.html"

$SystemsDir = Join-Path $RepoRoot "Doc\Systems"
$SystemsIndex = Join-Path $SystemsDir "README.md"
$SystemsFiles = Get-ChildItem $SystemsDir -Filter *.md | Where-Object { $_.Name -ne "README.md" } | Sort-Object Name | ForEach-Object { $_.FullName }

function Get-ShiftedMarkdown {
    param(
        [string]$Path,
        [int]$ShiftBy
    )

    $prefix = "#" * $ShiftBy
    $inCodeBlock = $false

    Get-Content -Path $Path -Encoding UTF8 | ForEach-Object {
        $line = $_
        if ($line -match '^\s*```') {
            $inCodeBlock = -not $inCodeBlock
            $line
        }
        elseif (-not $inCodeBlock -and $line -match '^#+\s') {
            $prefix + $line
        }
        else {
            $line
        }
    }
}

$Merged = New-Object System.Collections.Generic.List[string]
$Merged.Add((Get-Content -Path (Join-Path $RepoRoot "README.md") -Encoding UTF8 -Raw))
$Merged.Add((Get-Content -Path (Join-Path $RepoRoot "Doc\WORKFLOW.md") -Encoding UTF8 -Raw))
$Merged.Add((Get-Content -Path $SystemsIndex -Encoding UTF8 -Raw))

foreach ($file in $SystemsFiles) {
    $Merged.Add((Get-ShiftedMarkdown -Path $file -ShiftBy 1 | Out-String))
}

$TempFile = Join-Path ([System.IO.Path]::GetTempPath()) "mage-doc-merged.md"
Set-Content -Path $TempFile -Value ($Merged -join "`n`n") -Encoding UTF8

& pandoc $TempFile `
    --standalone `
    --embed-resources `
    --css (Join-Path $PSScriptRoot "doc-style.css") `
    --metadata title="Mage - Documentation consolidee" `
    --toc --toc-depth=3 `
    --syntax-highlighting=tango `
    -o $Out

Remove-Item -Path $TempFile -Force

Write-Host "Ecrit : $Out"
