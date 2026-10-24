$path = 'C:\Users\ASUS TUF GAMING\Desktop\stickrun\stickrun.swf'
$bytes = [System.IO.File]::ReadAllBytes($path)

# SWF ZLIB Header: ilk 8 bayttan sonra ZLIB baslar (RFC 1950 - 2 bayt zlib header'i atlanirsa raw deflate olur)
$stream = New-Object System.IO.MemoryStream(,$bytes)
$stream.Seek(10, [System.IO.SeekOrigin]::Begin) | Out-Null # 8 bayt header + 2 bayt ZLIB wrapper

$deflate = New-Object System.IO.Compression.DeflateStream($stream, [System.IO.Compression.CompressionMode]::Decompress)
$output = New-Object System.IO.MemoryStream
$deflate.CopyTo($output)
$decompressed = $output.ToArray()

Write-Host "Basariyla acildi! Acilmis boyut: $($decompressed.Length) bayt"

$ascii = [System.Text.Encoding]::ASCII.GetString($decompressed)
$keywords = @("playerio", "PlayerIO", "quickConnect", "authenticate", "developmentServer", "StickRun", "roomType", "connect")

foreach ($kw in $keywords) {
    $matches = [regex]::Matches($ascii, "$kw", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
    Write-Host "Kelime '$kw' eslesme sayisi: $($matches.Count)"
}

# Regex ile URL veya ID arama
$urlMatches = [regex]::Matches($ascii, 'https?://[a-zA-Z0-9_\-\./]+')
Write-Host "`nBulunan URL'ler:"
$urlMatches | Select-Object -First 10 | ForEach-Object { Write-Host " - " $_.Value }

# Game ID arama (PlayerIO Game ID format: stick-run-xxx veya benzeri)
$gameIds = [regex]::Matches($ascii, '[a-z0-9\-]{8,35}(?=:|\.playerio|gameid)')
Write-Host "`nOlası Game ID'ler:"
$gameIds | Select-Object -First 5 | ForEach-Object { Write-Host " - " $_.Value }
