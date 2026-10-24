$path = 'C:\Users\ASUS TUF GAMING\Desktop\stickrun\stickrun.swf'
$bytes = [System.IO.File]::ReadAllBytes($path)
$stream = New-Object System.IO.MemoryStream(,$bytes)
$stream.Seek(10, [System.IO.SeekOrigin]::Begin) | Out-Null
$deflate = New-Object System.IO.Compression.DeflateStream($stream, [System.IO.Compression.CompressionMode]::Decompress)
$output = New-Object System.IO.MemoryStream
$deflate.CopyTo($output)
$ascii = [System.Text.Encoding]::ASCII.GetString($output.ToArray())

Write-Host "=== PLAYERIO CONNECT CAGRISI BULUNUYOR ==="
$matches = [regex]::Matches($ascii, '(?i)(?:gameid|quickconnect|connect)\x00*([a-zA-Z0-9_\-]{6,40})')
$matches | Select-Object -First 20 | ForEach-Object { Write-Host "Parametre:" $_.Groups[1].Value }

# .swf icindeki tum string literallerini tara ve playerio gameid formatinda olanlari goster
Write-Host "`n=== OYUNUN KULLANDIGI PLAYERIO ID'LERI ==="
$ids = [regex]::Matches($ascii, '[a-z0-9\-]+-([a-z0-9]{20,30})')
$ids | Select-Object -First 10 | ForEach-Object { Write-Host "Bulunan ID:" $_.Value }

Write-Host "`n=== ODA (ROOM) TURLERI ==="
$rooms = [regex]::Matches($ascii, '(?i)roomtype\x00*([a-zA-Z0-9_]+)')
$rooms | Select-Object -First 10 | ForEach-Object { Write-Host "Oda:" $_.Groups[1].Value }
