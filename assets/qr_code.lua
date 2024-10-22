-- qr_code.lua
-- Generates and prints a QR code for a given text using luaqrcode

-- Include the QR Code library
local qrencode = dofile("/assets/qrencode.lua")

-- Text to encode (using lowercase as per your request)
local text = 'https://developer.espressif.com/tags/lua'

-- Generate the QR code
local ok, tab_or_message = qrencode.qrcode(text)
if not ok then
    print("Error generating QR code: " .. tab_or_message)
    return
end

local function matrix_to_string(tab, padding, padding_char, white_pixel, black_pixel)
    padding = padding or 1
    white_pixel = white_pixel or '  '
    black_pixel = black_pixel or '██'
    padding_char = padding_char or white_pixel

    local str_tab = {} -- Hold each row of the QR code in a table

    -- Add padding rows at the top
    for i = 1, padding do
        table.insert(str_tab, string.rep(padding_char, #tab + 2 * padding))
    end

    for y = 1, #tab do
        local line = string.rep(padding_char, padding)
        for x = 1, #tab do
            if tab[x][y] > 0 then
                line = line .. black_pixel
            else
                line = line .. white_pixel
            end
        end
        line = line .. string.rep(padding_char, padding)
        table.insert(str_tab, line)
    end

    -- Add padding rows at the bottom
    for i = 1, padding do
        table.insert(str_tab, string.rep(padding_char, #tab + 2 * padding))
    end

    return str_tab
end

-- Convert the QR code matrix to a string
local qr_lines = matrix_to_string(tab_or_message)

-- Print the QR code
print("QR Code for: " .. text)
for _, line in ipairs(qr_lines) do
    print(line)
end
