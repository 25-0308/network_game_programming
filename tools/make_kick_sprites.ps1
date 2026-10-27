# Builds assets/player_kick_left.png and player_kick_right.png:
# frame 0 of the idle body sheet plus a leg and pixel-art boot swinging forward.
#
# run from PowerShell in any folder: & <repo>\tools\make_kick_sprites.ps1
# frame size and count must match kickFrameCnt / the createAnimation call in Player.hpp
Add-Type -AssemblyName System.Drawing

$assets = [System.IO.Path]::GetFullPath( (Join-Path $PSScriptRoot '..\HeadSoccer\HeadSoccer\assets') )

$frameW = 123; $frameH = 61
$bodyX = 15; $bodyY = 22           # body (93x33) top-left inside a frame
$pivotX = 81.5; $pivotY = 33.5     # hip, = player pos + (20, 48) for the left-side player
$legLen = 20
$px = 3                             # art pixel size used by the existing sprites
$angles = @(40, 25, 0, -25, -40, -20, 10, 35)   # leg angle, 0 = straight forward, + = down

# boot drawn with toe toward +x and sole toward +y, origin at the ankle
$boot = @(
    '..WWW....',
    '.WRRWW...',
    'WWWWWWWW.',
    'WWWWWWWWW',
    'SSSSSSSSS'
)
$colors = @{
    'W' = [System.Drawing.Color]::FromArgb(255, 240, 240, 240)
    'R' = [System.Drawing.Color]::FromArgb(255, 210, 40, 40)
    'S' = [System.Drawing.Color]::FromArgb(255, 35, 35, 35)
}
$legColor = [System.Drawing.Color]::FromArgb(255, 45, 45, 45)

function New-LegLayer($angle) {
    $layer = New-Object System.Drawing.Bitmap $frameW, $frameH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($layer)
    $g.SmoothingMode = 'None'
    $g.PixelOffsetMode = 'Half'
    $g.Clear([System.Drawing.Color]::Transparent)

    $rad = $angle * [Math]::PI / 180
    $endX = $pivotX + $legLen * [Math]::Cos($rad)
    $endY = $pivotY + $legLen * [Math]::Sin($rad)

    $pen = New-Object System.Drawing.Pen $legColor, 7
    $g.DrawLine($pen, [single]$pivotX, [single]$pivotY, [single]$endX, [single]$endY)
    $pen.Dispose()

    # boot stands upright when the leg points down (90), so rotate by angle - 90
    $g.TranslateTransform([single]$endX, [single]$endY)
    $g.RotateTransform([single]($angle - 90))
    for ($row = 0; $row -lt $boot.Count; $row++) {
        for ($col = 0; $col -lt $boot[$row].Length; $col++) {
            $ch = [string]$boot[$row][$col]
            if ($ch -eq '.') { continue }
            $brush = New-Object System.Drawing.SolidBrush $colors[$ch]
            $g.FillRectangle($brush, [single](($col - 3) * $px), [single]($row * $px - 2), [single]$px, [single]$px)
            $brush.Dispose()
        }
    }
    $g.Dispose()
    return $layer
}

function Build-Sheet($bodyFile, $outFile, $mirror) {
    $bodySheet = [System.Drawing.Bitmap]::FromFile("$assets\$bodyFile")
    $sheet = New-Object System.Drawing.Bitmap ($frameW * $angles.Count), $frameH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($sheet)
    $g.Clear([System.Drawing.Color]::Transparent)
    $g.InterpolationMode = 'NearestNeighbor'
    $g.PixelOffsetMode = 'Half'

    for ($i = 0; $i -lt $angles.Count; $i++) {
        $leg = New-LegLayer $angles[$i]
        if ($mirror) { $leg.RotateFlip([System.Drawing.RotateFlipType]::RotateNoneFlipX) }

        # leg behind the body, so the hip joint is hidden
        $g.DrawImage($leg, $i * $frameW, 0)
        $g.DrawImage($bodySheet, (New-Object System.Drawing.Rectangle ($i * $frameW + $bodyX), $bodyY, 93, 33),
            (New-Object System.Drawing.Rectangle 0, 0, 93, 33), [System.Drawing.GraphicsUnit]::Pixel)
        $leg.Dispose()
    }

    $g.Dispose()
    $sheet.Save("$assets\$outFile", [System.Drawing.Imaging.ImageFormat]::Png)
    $sheet.Dispose()
    $bodySheet.Dispose()
}

Build-Sheet 'player_idle_left.png' 'player_kick_left.png' $false
Build-Sheet 'player_idle_right.png' 'player_kick_right.png' $true
'ok'
