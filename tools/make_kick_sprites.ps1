# Builds assets/player_kick_left.png and player_kick_right.png from frame 0 of the idle body sheet:
# the front shoe is cut out of the body and swung forward and up, the rest of the body stays put.
#
# run from PowerShell in any folder: & <repo>\tools\make_kick_sprites.ps1
# frame size and count must match kickFrameCnt / the createAnimation call in Player.hpp
Add-Type -AssemblyName System.Drawing

$assets = [System.IO.Path]::GetFullPath( (Join-Path $PSScriptRoot '..\HeadSoccer\HeadSoccer\assets') )

$frameW = 123; $frameH = 61
$bodyX = 15; $bodyY = 22           # body (93x33) top-left inside a frame
$bodyW = 93; $bodyH = 33

# front shoe inside the left-side body frame (the black shoe on the right half)
$shoe = New-Object System.Drawing.Rectangle 47, 18, 34, 15

# per frame: shoe move (dx, dy) and rotation in degrees, negative = toe up
$keys = @(
    @(-3, -2, 8),
    @(4, -2, -10),
    @(12, -6, -30),
    @(18, -12, -50),
    @(20, -16, -60),
    @(16, -11, -45),
    @(8, -5, -20),
    @(2, -1, -5)
)

function Build-Sheet($bodyFile, $outFile, $mirror) {
    $bodySheet = [System.Drawing.Bitmap]::FromFile("$assets\$bodyFile")

    # work on the left-facing body; the right-side sheet is the mirrored result
    $body = $bodySheet.Clone( (New-Object System.Drawing.Rectangle 0, 0, $bodyW, $bodyH), [System.Drawing.Imaging.PixelFormat]::Format32bppArgb )
    if ($mirror) { $body.RotateFlip([System.Drawing.RotateFlipType]::RotateNoneFlipX) }

    $shoeImg = $body.Clone($shoe, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    for ($y = $shoe.Top; $y -lt $shoe.Bottom; $y++) {
        for ($x = $shoe.Left; $x -lt $shoe.Right; $x++) {
            $body.SetPixel($x, $y, [System.Drawing.Color]::Transparent)
        }
    }

    $sheet = New-Object System.Drawing.Bitmap ($frameW * $keys.Count), $frameH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($sheet)
    $g.Clear([System.Drawing.Color]::Transparent)
    $g.InterpolationMode = 'NearestNeighbor'
    $g.PixelOffsetMode = 'Half'

    $shoeCx = $bodyX + $shoe.Left + $shoe.Width / 2.0
    $shoeCy = $bodyY + $shoe.Top + $shoe.Height / 2.0

    for ($i = 0; $i -lt $keys.Count; $i++) {
        $frame = New-Object System.Drawing.Bitmap $frameW, $frameH, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $fg = [System.Drawing.Graphics]::FromImage($frame)
        $fg.Clear([System.Drawing.Color]::Transparent)
        $fg.InterpolationMode = 'NearestNeighbor'
        $fg.PixelOffsetMode = 'Half'

        $fg.DrawImage($body, $bodyX, $bodyY, $bodyW, $bodyH)

        $dx, $dy, $rot = $keys[$i]
        $fg.TranslateTransform([single]($shoeCx + $dx), [single]($shoeCy + $dy))
        $fg.RotateTransform([single]$rot)
        $fg.DrawImage($shoeImg, [single](-$shoe.Width / 2.0), [single](-$shoe.Height / 2.0), [single]$shoe.Width, [single]$shoe.Height)
        $fg.Dispose()

        if ($mirror) { $frame.RotateFlip([System.Drawing.RotateFlipType]::RotateNoneFlipX) }
        $g.DrawImage($frame, $i * $frameW, 0, $frameW, $frameH)
        $frame.Dispose()
    }

    $g.Dispose()
    $sheet.Save("$assets\$outFile", [System.Drawing.Imaging.ImageFormat]::Png)
    $sheet.Dispose()
    $shoeImg.Dispose()
    $body.Dispose()
    $bodySheet.Dispose()
}

Build-Sheet 'player_idle_left.png' 'player_kick_left.png' $false
Build-Sheet 'player_idle_right.png' 'player_kick_right.png' $true
'ok'
