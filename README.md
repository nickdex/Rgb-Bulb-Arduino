
# RGB Bulb Arduino client

A node server which controls and RGB bulb. It is done by using slider for Red, Green and Blue colors respectively, along with a hexagon of colors

## Communicating with Pi/Arduino

To send rbg values to IoT device, use following schema to send data by PubNub
```
{
  type: 'type', // Can be 'rgb' or 'button'
  value: data // JSON, containing rgb values in JSON; or bool values for button
}
```

## Built With

* [Node](https://nodejs.org/en/) - The web server used
* [PubNub](https://www.pubnub.com/) - Messaging service to communicate b/w Pi and Server
* [Material Design for Bootstrap](https://mdbootstrap.com/) - Layout and components
* [Range Slider](http://propeller.in/components/range-slider.php) - Sliders for R/G/B colors

## Authors

* **Nikhil Warke**

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone who's code was used
