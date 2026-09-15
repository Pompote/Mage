# Doc/Menus/build-menu.ps1
# Compile MainMenu.html (source editable, qui reference css/*.css et js/*.js par des balises
# <link>/<script src>) en un seul fichier HTML autonome, pour un partage facile (email, ticket,
# copie ailleurs) sans trainer les fichiers css/js a cote. Chaque <link rel="stylesheet"
# href="css/X.css"> est remplace par son contenu inline dans <style>, chaque
# <script src="js/X.js"></script> par son contenu inline dans <script>. Le fichier source
# (MainMenu.html) n'est jamais modifie -- seule la sortie (MainMenu.compiled.html, regenerable,
# non commitee, voir .gitignore) est ecrite.
#
# Contrairement a Doc/build-doc.ps1 (qui appelle Pandoc), aucune dependance externe ici -- simple
# substitution de texte, les tags recherches doivent correspondre exactement a ceux ecrits dans
# MainMenu.html.

$ErrorActionPreference = "Stop"
$Root = $PSScriptRoot
$SourceFile = Join-Path $Root "MainMenu.html"
$Out = Join-Path $Root "MainMenu.compiled.html"

$CssFiles = @("css/tokens.css", "css/layout.css", "css/room.css", "css/spells.css", "css/kit.css")
$JsFiles  = @("js/background.js", "js/spell-data.js", "js/navigation.js", "js/room.js", "js/spells.js", "js/kit.js")

$html = Get-Content -Path $SourceFile -Encoding UTF8 -Raw

foreach ($relative in $CssFiles) {
    $tag = '<link rel="stylesheet" href="{0}">' -f $relative
    $content = Get-Content -Path (Join-Path $Root $relative) -Encoding UTF8 -Raw
    if (-not $html.Contains($tag)) { throw "Tag introuvable dans MainMenu.html : $tag" }
    $html = $html.Replace($tag, "<style>`n$content`n</style>")
}

foreach ($relative in $JsFiles) {
    $tag = '<script src="{0}"></script>' -f $relative
    $content = Get-Content -Path (Join-Path $Root $relative) -Encoding UTF8 -Raw
    if (-not $html.Contains($tag)) { throw "Tag introuvable dans MainMenu.html : $tag" }
    $html = $html.Replace($tag, "<script>`n$content`n</script>")
}

Set-Content -Path $Out -Value $html -Encoding UTF8 -NoNewline

Write-Host "Ecrit : $Out"
