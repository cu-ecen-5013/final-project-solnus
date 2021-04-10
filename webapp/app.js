var express = require("express");
var path = require("path");
var routes = require("./routes");
var bodyParser = require('body-parser');
var app = express();
var urlencodedParser = bodyParser.urlencoded({ extended: false });
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

app.set("port",process.env.PORT || 3000);
app.set("views", path.join(__dirname, "views"));
app.set("view engine", "ejs");

app.use(routes);
app.listen(app.get("port"),function(){
    console.log("server started on port "+ app.get("port"));
});