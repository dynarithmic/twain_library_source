open System
open System.Runtime.InteropServices
open dtwainapi
open System.Text

[<EntryPoint>]
let main argv =

    let dllname =
        if Environment.Is64BitProcess then
            "dtwain64u.dll"
        else
            "dtwain32u.dll"

    // Load the DLL first — required before any DTWAIN calls
    TwainAPI.Load dllname
    printfn "DTWAIN DLL loaded successfully."

    let exitCode =
        try
            let initResult = TwainAPI.DTWAIN_SysInitialize()
            if initResult = 0 then 
                printfn "DTWAIN_SysInitialize returned: 0x%08X" (initResult.ToInt64())
                1
            else
                let sourceResult = TwainAPI.DTWAIN_SelectSource2 IntPtr.Zero "Select Source" 0 0 TwainAPI.DTWAIN_DLG_CENTER_SCREEN 
                if sourceResult = 0 then 
                    printfn "No TWAIN Source was selected"
                    TwainAPI.DTWAIN_SysDestroy() |> ignore
                    1
                else
                    let prodname = new StringBuilder(256)
                    let ret = TwainAPI.DTWAIN_GetSourceProductNameW sourceResult prodname 256
                    printfn "The name of the selected TWAIN Source is: %s" (prodname.ToString())

                    // Example usage of DTWAIN_ARRAY:
                    // Get the device capabilities supported by the device
               
                    // Note: The DTWAIN_ARRAY, DTWAIN_SOURCE, DTWAIN_FRAME, and DTWAIN_RANGE are actually void pointers
                    // so you have to declare them as IntPtr.Zero if a DTWAIN function requires a parameter to be of this type.
                    let mutable cap_array = TwainAPI.DTWAIN_EnumSupportedCapsEx2 sourceResult 

                    // Get the number of items in the array
                    let mutable arrcount = TwainAPI.DTWAIN_ArrayGetCount cap_array
                    printfn "There are %d capabilities defined for device %s" (arrcount) (prodname.ToString())

                    // print each capability
                    let mutable long_val = 0
                    for i = 1 to arrcount do
                        let index = i - 1
                        TwainAPI.DTWAIN_ArrayGetAtLong cap_array index &long_val |> ignore
                        TwainAPI.DTWAIN_GetNameFromCap long_val prodname 256 |> ignore
                        printfn "Capability %d: %s  Value: %d" (i) (prodname.ToString()) (long_val)

                    // Destroy the array when done
                    TwainAPI.DTWAIN_ArrayDestroy cap_array |> ignore
                    let mutable status_ = 0
                    // Now Acquire to a BMP file
                    TwainAPI.DTWAIN_AcquireFile sourceResult "C:/Saved_images/TEST.BMP" TwainAPI.DTWAIN_BMP 
                                               TwainAPI.DTWAIN_USELONGNAME
                                               TwainAPI.DTWAIN_PT_DEFAULT 1 1 1 &status_ |> ignore

                    TwainAPI.DTWAIN_SysDestroy() |> ignore
                    0  // success
        with
        | ex ->
            printfn "Error: %s" ex.Message
            1  // failure

    // Unload the DLL before exiting (no 'finally' used)
    TwainAPI.Unload()
    printfn "DLL unloaded."
    printfn "%d" TwainAPI.DTWAIN_BMP

    // Return the appropriate exit code
    exitCode