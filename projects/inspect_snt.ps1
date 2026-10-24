$path = 'C:\Users\ASUS TUF GAMING\Desktop\Stick N Trade 1.6 Summer Edition.app\~stickrun.swf'
$bytes = [System.IO.File]::ReadAllBytes($path)
$stream = New-Object System.IO.MemoryStream(,$bytes)
$stream.Seek(10, [System.IO.SeekOrigin]::Begin) | Out-Null
$deflate = New-Object System.IO.Compression.DeflateStream($stream, [System.IO.Compression.CompressionMode]::Decompress)
$output = New-Object System.IO.MemoryStream
$deflate.CopyTo($output)
$ascii = [System.Text.Encoding]::ASCII.GetString($output.ToArray())

Write-Host "=== STICK N TRADE (YENI SURUM) PLAYERIO ID'LERI ==="
$ids = [regex]::Matches($ascii, '[a-z0-9\-]+-([a-z0-9]{20,30})')
$ids | Select-Object -First 10 | ForEach-Object { Write-Host "Bulunan ID:" $_.Value }

Write-Host "`n=== BAGLANDIKLARI SUNUCU VEYA WEB ADRESLERI ==="
$urls = [regex]::Matches($ascii, 'https?://[a-zA-Z0-9_\-\./]+')
$urls | Select-Object -First 10 | ForEach-Object { Write-Host "URL:" $_.Value }
