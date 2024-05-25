presetSelect = document.getElementById("presets");

presetSelect.addEventListener("change", function(){
    var colorElements = document.querySelectorAll(".jscolor");
    var resultingFunction = "";

    colorElements.forEach(function(element){
        var color = element.style.backgroundColor;
        var formattedFunction = color.replace("rgb(", "AddColorRGB(reference, ");

        resultingFunction += formattedFunction + ";\n";
    });

    console.log(resultingFunction);
});